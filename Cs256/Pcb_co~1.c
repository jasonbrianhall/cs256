/*	Author: Wes Queen, Kenneth Heck, Jason Hall
	Filename: pcb_cont.c
	Last Modified: April 15, 2001

	Purpose: Provide a data structure to represent a PCB and to manage
	collections of related PCBs. Also provide a structure for process
	queues.

	This also provides a set of procedures to allocate and initialize
	PCBs, to search for a specific PCB based on its content, and to
	manipulate the process queues.

	An additional set of commands are called from COMHAN.	These commands
	will be used to invoke and exercise the control procedures and also
	be used to display the contents of a specified PCB or set of PCBs.

	Also contains the interrupt handler and process dispatcher.

	*/


#include "Pcb_cont2.h"
#include "mpx_supt.h"
#include "ioschd.h"
#include "trmdrive.h"
#include "comhand.h"
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#define Max_PCB 20

static char PCB_Names[Max_PCB][15];
static int Num_PCB = 0;
// Declaration and initialization of addresses
unsigned short ss_save = NULL, sp_save = NULL;
/* Declaration and initialization of three queues */
Header Ready_queue, Blocked_queue, Ready_susp_queue,Blocked_susp_queue;
PCB *cop;
context *context_temp;
parameters *param_p;

extern IOCB com_control_block;
extern IOCB print_control_block;
extern IOCB term_control_block;

int initialize_PCB()
{
Ready_queue.Start = NULL;
Ready_queue.End = NULL;
Blocked_queue.Start = NULL;
Blocked_queue.End = NULL;
Ready_susp_queue.Start = NULL;
Ready_susp_queue.End = NULL;

return 0;
}

/* Allocate memory for a new PCB */
PCB * Allocate_PCB()
{

	/* Initialize pointer variables */
	PCB *PCB_Pointer;

	/* Check for maximum number of PCBs, if exceeded create error */
	if (Num_PCB == Max_PCB)
	{
		printf("ERROR: Maximum num of PCB's, %i, has been exceeded\n",Max_PCB);
		return NULL;
	}

	/* Allocate memory for a new PCB */
	PCB_Pointer = (PCB*) sys_alloc_mem(sizeof(PCB));

	/* increment the number of PCB's */
	Num_PCB++;

	return PCB_Pointer;
}

/* Deallocate and free the memory of a PCB */
void Free_PCB(PCB * p_PCB) {

	/* Initialize variables */
	int error, i, check, position;
	char Name[15];

	/* Copy process name to temparary variable */
	strcpy(Name,p_PCB->Process_Name);

	/* Deallocate memory for the PCB */
	error = sys_free_mem((void *) p_PCB);
	if (error != 0) {
		printf("unable to deallocate pointer,please try again");
		return;
	}

	/* Find process name from the list */
	for (i=0; i<Num_PCB; i++) {
		check = strcmp(Name,PCB_Names[i]);
		if (check == 0) {
			position = i;
			break;
		}
	}

	/* Remove process name from the list */
	for (i=position; i<Num_PCB; i++) {
		if (i == Num_PCB-1) {
			strcpy(PCB_Names[Num_PCB-1],"\0");
		}
		else {
			strcpy(PCB_Names[i],PCB_Names[i+1]);
		}
	}

	Num_PCB--;
}

/* Setup values for newly created PCB */
int Setup_PCB(PCB * p_PCB, char p_Name[15], int p_Process_Class,
				  int p_Priority, int p_State[2]) {

	int i, check;


	/* Check for valid PCB pointer */
	if (p_PCB == NULL) {
		return (-1);
	}


	/* Check for valid name, making sure the name has not already been stored */
	for (i=0; i<Num_PCB; i++) {
		check = strcmp(p_Name,PCB_Names[i]);
		if (check == 0) {

		printf("name already used for this PCB please choose another\n");
		return 9;
		}
	}

	strcpy(p_PCB->Process_Name,p_Name);
	strcpy(PCB_Names[Num_PCB-1],p_Name);

	/* Check for valid class */
	if (p_Process_Class != SYSTEM_P && p_Process_Class != APPLICATION_P) {
		printf("not a valid process class, must either choose system or application\n");
	}

	p_PCB->Process_Class = p_Process_Class;

	// Check for valid priority
	if (p_Priority < (-128) || p_Priority > 127) {
		printf("not a valid priority, please choose a number between -128 and 127\n");
	}
	p_PCB->Process_Priority = p_Priority;

	/* Check for valid state */
	if ((p_State[0] != RUNNING && p_State[0] != READY) ||
		 (p_State[1] != NOT_SUSPENDED && p_State[1] != SUSPENDED)) {
		printf("not a valid state, please enter correct state\n");
	}
	p_PCB->Process_State[0] = p_State[0];
	p_PCB->Process_State[1] = p_State[1];
	p_PCB->Process_Mem_Size = 0;
	p_PCB->Process_Stack_Size = 1024;
	p_PCB->Load_Address = NULL;

	p_PCB->Exec_Address = NULL;
	return 0;
}

// Find a specific PCB in a specific Queue
PCB* Find_PCB(Header * FindQ, char Name[15]) {

	// Initialize variables
	int Temp;
	Process_List *QStart;

	// Check for valid Header pointer
	if (FindQ == NULL) {
		printf("PCB pointer input to find queue not valid\n");
		return NULL;
	}

	QStart = FindQ->Start;

	// Search through Queue to find a matching PCB name
	while (QStart != NULL){


		Temp = strcmp(strtok(Name,"\n"), (QStart->Item->Process_Name));
		if (Temp == 0){
			return QStart->Item;
		}
		QStart = QStart->Next;
	}
	return NULL;
}


int Insert_PCB(PCB * NewPCB, Header * InsertQ, int sh_t)
{
unsigned int * Address_Pointer;
	// Initialize variables
	Process_List *Temp, *Temp2, *QStart;
	PCB *Temp_Swap;
	int error, tempchecker;
	int returned;



	// Check for valid PCB pointer
	if (NewPCB == NULL) {
		printf("PCB pointer input to find PCB not valid\n");
		return NULL;
	}

	// Check for valid Header pointer
	if (InsertQ == NULL)
	{
		printf("Queue requested to insert not a valid header pointer\n");
		return NULL;
	}

	// Check for FIFO or Priority
	if (sh_t < 0 || sh_t > 1)
	{
		printf("Not a valid request for priority, should be FIFO or PRIOIRITY\n");
		return NULL;
	}

	// Check for an empty Queue
	if ((InsertQ->Start == NULL) && (InsertQ->End == NULL))
	{
		QStart = (Process_List *) sys_alloc_mem(sizeof(Process_List));

		InsertQ->Start=QStart;
		InsertQ->End=QStart;

		InsertQ->Start->Item=NewPCB;
		InsertQ->Start->Next=NULL;
		InsertQ->Start->Previous=NULL;


	}
	// Allocate memory for a new node and setup pointers
	else
	{

		if(sh_t>=0 && sh_t<=1)
		{

			if(sh_t==1)
			{
			Temp = (Process_List *) sys_alloc_mem(sizeof(Process_List));
				if (Temp == NULL)
				{
					printf("unable to allocate memory for Temp PCB\n");
					return NULL;
				}
				InsertQ->End->Next=Temp;
				tempchecker=0;
				do
				{

					if(Temp->Next==NULL)
					{
						tempchecker=1;
						Temp->Item=NewPCB;
						Temp->Previous=InsertQ->End;
						Temp->Next=NULL;
						InsertQ->End=Temp;
					}
					if(Temp->Next!=NULL)
					{
						Temp=Temp->Next;
					}
				}while(tempchecker==0);
			}


			if(sh_t==0)
			{
			Temp = (Process_List *) sys_alloc_mem(sizeof(Process_List));
				if (Temp == NULL)
				{
					printf("unable to allocate memory for Temp PCB\n");
					return NULL;
				}
				tempchecker=0;
				Temp2=InsertQ->Start;
				do
				{
					if(NewPCB->Process_Priority>Temp2->Item->Process_Priority)
					{
						tempchecker=1;

						if(Temp2->Previous==NULL)
						{

							InsertQ->Start->Previous=Temp;
							Temp->Item=NewPCB;
							Temp->Next=InsertQ->Start;
							Temp->Previous=NULL;
							InsertQ->Start=Temp;
						}
						else
						{
							Temp->Item=NewPCB;
							Temp->Previous=Temp2->Previous;
							Temp->Next=Temp2;
							Temp2->Previous=Temp;
						}
					}
                                        else
                                        {
					        if(Temp2->Next==NULL && tempchecker==0)
					        {
						        tempchecker=1;
						        Temp2->Next=Temp;
						        Temp->Item=NewPCB;
						        Temp->Previous=Temp2;
						        Temp->Next=NULL;
						        InsertQ->End=Temp;
					        }
                                                else
                                                {
                                                        if(Temp2->Next!=NULL)
					                {
						                Temp2=Temp2->Next;
					                }
                                                }
                                        }
				}while(tempchecker==0);


			}
		}




	return 0;
	}




	return 0;
}

// Remove a specific PCB from a specific Queue
int Remove_PCB(Header * RemoveQ, char Name[15]) {

	// Initialize variables
	int Temp, error;
	Process_List *QStart;

	// Check for valid Header pointer
	if (RemoveQ == NULL) {
		printf("PCB pointer input to find PCB not valid\n");
	}

	QStart = RemoveQ->Start;

	// Search through Queue to find a matching PCB name
	while (QStart != NULL) {

		// Compare two strings

		if (strcmp(Name, QStart->Item->Process_Name)== 0)
		{

			// Check for start
			if (QStart->Previous == NULL)
			{

				// Remove node
				RemoveQ->Start = QStart->Next;
				if (QStart->Next == NULL) {
					RemoveQ->End = NULL;
				}
				RemoveQ->Start->Previous = NULL;
				error = sys_free_mem((void *) QStart);
				if (error != 0) {
					printf("error freeing PCB from memory\n");
				}
				return 0;
			}

			// Check for end
			else if (QStart->Next == NULL) {

				// Remove node
				RemoveQ->End = QStart->Previous;
				if (QStart->Previous == NULL) {
					RemoveQ->Start = NULL;
				}
				RemoveQ->End->Next = NULL;
				error = sys_free_mem((void *) QStart);
				if (error != 0) {
					return error;
				}
				return 0;
			}
			else
			{

				// Remove node
				QStart->Next->Previous = QStart->Previous;
				QStart->Previous->Next = QStart->Next;
				error = sys_free_mem((void *) QStart);
				if (error != 0) {
					printf("error freeing PCB from memory\n");
				}
				return 0;
			}
		}

		// Increment the Queue
		QStart = QStart->Next;
	}
	return -9;
}


 // Create a new PCB
int Create_known_pcb(char pname[15],int prty) {

	// Initialize variables
	PCB *TempPCB;
	char ProcessName[15];
	int error, PClass, PPriority,
		 PState[2];

	// Allocate memory for a PCB
	TempPCB = Allocate_PCB();
	if (TempPCB == NULL) {
		printf("Unable to create new PCB, number of PCB's reached max\n");
		return 0;
	}


	strtok(pname,"\n");


	// Initialize the process state
	PState[0] = READY;
	PState[1] = NOT_SUSPENDED;

	// Setup values for a PCB
	error = Setup_PCB(TempPCB, pname, SYSTEM_P, prty, PState);
	if (error != 0) {
		printf("error in setup of PCB\n");
		return NULL;
	}


	error = Insert_PCB(TempPCB, &Ready_queue, PRIORITYQ);
	if (error != 0) {
		printf("error inserting new PCB %s into queue\n",pname);
		return NULL;
	}



	printf("Process '%s' created sucessfully\n",pname);
	return 0;
}

int Create_PCB(char pname[15]) {

	// Initialize variables
	PCB *TempPCB;
	int queue_type=1000;
	char ProcessName[15],cmndin[4];
	int Class_Name=0, error, flag=0, PClass, PPriority,
		 PState[2];

	// Allocate memory for a PCB
	TempPCB = Allocate_PCB();
	if (TempPCB == NULL) {
		printf("Unable to create new PCB, number of PCB's reached max\n");
		return 0;
	}

	// Get name of process
	strtok(pname,"\n");
	// Get the process class
	while (flag == 0) {
		printf("Enter the process class:\n");
		printf("Enter a '1' for APPLICATION or '2' for SYSTEM)\n");
		fflush(stdin);
		scanf("%d",&Class_Name);

		// Check the class
		if (Class_Name == 1) {
			PClass = APPLICATION_P;
			flag = 1;
		}
		else if (Class_Name == 2) {
			PClass = SYSTEM_P;
			flag = 1;
		}
		else {
			printf("Invalid process class, please enter '1' or '2' \n");
		}
	}
	flag = 0;

	// Get the process priority
	while (flag == 0) {
		printf("Enter the process priority (Between -128 and +127):\n");
		fflush(stdin);
		scanf("%d",&PPriority);
		//sys_req(READ,TERMINAL,cmndin,&comsz);
		// Check the priority
		//PPriority = atoi(cmndin);
		printf("%i\n",PPriority);
		//printf("char %s\n,",cmndin);
		if (PPriority < (-128) || PPriority > (127)) {
			printf("Invalid process priority,choose number between -128 and 127 \n");
		}
		else {
			queue_type = PRIORITYQ;
			flag = 1;
		}
		}


	// Initialize the process state
	PState[0] = READY;
	PState[1] = NOT_SUSPENDED;

	// Setup values for a PCB
	error = Setup_PCB(TempPCB, pname, PClass, PPriority, PState);
	if (error != 0) {
		printf("error in setup of PCB\n");
		return NULL;
	}


	error = Insert_PCB(TempPCB, &Ready_queue, queue_type);
	if (error != 0) {
		printf("error inserting new PCB %s into queue\n",pname);
		return NULL;
	}



	printf("Process '%s' created sucessfully\n",pname);
	return 0;
}

// Delete a PCB
int Delete_PCB() {

	// Initialize variables
	char ProcessName[15];
	int name_size=15, error;
	PCB *TempPCB;

	// Get name of process
	printf("Enter the process name (Maximum of 14 characters): ");
	fflush(stdin);
	error = sys_req(READ, TERMINAL, ProcessName, &name_size);
	if (error < 0) {
		printf("error reading input, please try again");
		return 0;
	}
	strtok(ProcessName,"\n");

	//Look in Ready Queue

	string_upper(ProcessName);
		TempPCB = Find_PCB(&Ready_queue, ProcessName);

	if (TempPCB != NULL) {

		error = Remove_PCB(&Ready_queue, ProcessName);
		if (error < 0) {
			printf("error removing PCB %s from ready queue\n",ProcessName);
			return 0;
		}
		Free_PCB(TempPCB);
		printf("Process '%s' deleted sucessfully\n",ProcessName);
		return 0;
	}

	// Look in the Blocked_queue




	TempPCB = Find_PCB(&Blocked_queue, ProcessName);

	if (TempPCB != NULL) {

		error = Remove_PCB(&Blocked_queue, ProcessName);
		if (error < 0) {
			printf("error removing PCB %s from blocked queue.\n",ProcessName);
			return 0;
		}
		Free_PCB(TempPCB);
		printf("Process '%s' deleted sucessfully\n",ProcessName);
		return 0;
	}

	// Look in the Ready_susp_queue

	TempPCB = Find_PCB(&Ready_susp_queue, ProcessName);
	if (TempPCB != NULL) {
		error = Remove_PCB(&Ready_susp_queue, ProcessName);
		if (error < 0) {
			printf("Error removing PCB %s from Ready_susp_queue\n",ProcessName);
			return 0;
		}
		Free_PCB(TempPCB);
		printf("Process '%s' deleted sucessfully\n",ProcessName);
		return 0;
	}
	else {
		printf("Process '%s' does not exist\n",ProcessName);
		return 0;
	}
}

// Block an existing PCB
int Block_PCB(char *ProcessName) {

	// Initialize variables
	//char ProcessName[15];
	int error;
	PCB *TempPCB;


	strtok(ProcessName,"\n");

	/* now find the process to be blocked.  Look in the ready queue to find the
	   process to be blocked */

	TempPCB = Find_PCB(&Ready_susp_queue, ProcessName);
	if (TempPCB != NULL) {
		TempPCB->Process_State[0] = BLOCKED;
		error = Insert_PCB(TempPCB, &Blocked_queue, FIFOQ);
		if (error != 0) {
			printf("error inserting PCB %s into blocked queue\n",ProcessName);
		}

		printf("Process '%s' blocked sucessfully\n",ProcessName);
		error = Remove_PCB(&Ready_susp_queue, ProcessName);
		if (error < 0) {
			printf("error removing PCB %s from Ready Suspended queue\n",ProcessName);
			return 0;
		}

		return 0;
	}
	// set up TempPCB to hold the pointer when found

	TempPCB = Find_PCB(&Ready_queue, ProcessName);

	if (TempPCB != NULL) {

		TempPCB->Process_State[0] = BLOCKED;

		error = Insert_PCB(TempPCB, &Blocked_queue, FIFOQ);

		if (error != 0) {
			printf("error inserting PCB %s into blocked queue\n",ProcessName);
		}

		printf("Process '%s' blocked sucessfully\n",ProcessName);
		error = Remove_PCB(&Ready_queue, ProcessName);
		if (error < 0) {
			printf("error removing PCB %s from Ready queue\n",ProcessName);
			return 0;
		}
		/* no priority for a block PCB so put it in the FIFO */


		return 0;
		}
	if (TempPCB == NULL) {
		printf("Cannot find PCB name %s in queues, does not exist\n",ProcessName);
		return 0;
	}
	printf("Process blocked suscessfully\n");
	return 0;
}

/* function unblockes PCB */

int Unblock_PCB(char *ProcessName) {

	// Initialize variables
	int error;
	PCB *TempPCB;

	// Get name of process
	strtok(ProcessName,"\n");

	/* look in the Blocked queue for the PCB and move to ready queue */

	TempPCB = Find_PCB(&Blocked_queue, ProcessName);
	if (TempPCB != NULL) {
		error = Remove_PCB(&Blocked_queue, ProcessName);
		if (error < 0) {
			printf("Error removing PCB %s from blocked queue\n",ProcessName);
			return 0;
		}
		TempPCB->Process_State[0] = READY;
		if (TempPCB->Process_State[1] == NOT_SUSPENDED) {
			error = Insert_PCB(TempPCB, &Ready_queue, PRIORITYQ);
			if (error != 0) {
				printf("error inserting PCB %s into ready queue\n",ProcessName);
				return 0;
			}
		}
		if (TempPCB->Process_State[1] == SUSPENDED){
			error = Insert_PCB(TempPCB, &Ready_susp_queue, FIFOQ);
			 if (error != 0) {
				printf("error inserting PCB %s into ready suspended queue\n",ProcessName);
				return 0;
			}
		}

	}

	/* check the Ready Queue to see if the PCB is already unblocked */

	TempPCB = Find_PCB(&Ready_queue, ProcessName);
	if (TempPCB != NULL) {
		return 0;
	}
	/* look in ready_susp_queue to see if PCB is already unblocked */

	TempPCB = Find_PCB(&Ready_susp_queue, ProcessName);
	if (TempPCB != NULL) {
		printf("Process '%s' is already unblocked\n",ProcessName);
		return 0;
	}
	else {
		printf("Process '%s' does not exist\n",ProcessName);
		return 0;
	}
}



/* Function will suspend a PCB */

int Suspend_PCB(char ProcessName[15]) {

	// Initialize variables
	int error;
	PCB *TempPCB;

	strtok(ProcessName,"\n");

	/* find the process first look in ready queue */

	TempPCB = Find_PCB(&Ready_queue, ProcessName);
	if (TempPCB != NULL) {
		if (TempPCB->Process_State[1] == SUSPENDED) {
			printf("Process '%s' already suspended-ready\n",ProcessName);
		}
		else {
			TempPCB->Process_State[1] = SUSPENDED;
			error = Remove_PCB(&Ready_queue, ProcessName);
			if (error != 0) {
				printf("error removing PCB %s from ready queue\n",ProcessName);
			}
			error = Insert_PCB(TempPCB, &Ready_susp_queue, PRIORITYQ);
			if (error != 0) {
				printf("error inserting PCB %s into ready-suspended queue\n",ProcessName);
			}
			printf("Process %s suspended-ready sucessfully\n",ProcessName);
		}
		return 0;
	}
	/* look in Ready_susp_queue */

	TempPCB = Find_PCB(&Ready_susp_queue, ProcessName);
	if (TempPCB != NULL) {
		if (TempPCB->Process_State[1] == SUSPENDED) {
			printf("Process '%s' already suspended-ready\n",ProcessName);
		}
		else {
			TempPCB->Process_State[1] = SUSPENDED;
			error = Remove_PCB(&Ready_susp_queue, ProcessName);
			if (error != 0) {
				printf("error removing PCB %s from ready-suspended queue",ProcessName);
			}
			error = Insert_PCB(TempPCB, &Ready_susp_queue, PRIORITYQ);
			if (error != 0) {
				printf("error inserting PCB %s into ready-suspended queue\n",ProcessName);
			}
			printf("Process '%s' suspended-ready sucessfully\n",ProcessName);
		}
		return 0;
	}

	/* look in blocked queue for PCB */

	TempPCB = Find_PCB(&Blocked_queue, ProcessName);
	if (TempPCB != NULL) {
		if (TempPCB->Process_State[1] == SUSPENDED) {
			printf("Process '%s' already suspended-blocked\n",ProcessName);
		}
		else {
			TempPCB->Process_State[1] = SUSPENDED;
			printf("Process '%s' suspended-blocked sucessfully\n",ProcessName);
		}
		return 0;
	}



	else {
		printf("Process '%s' does not exist\n",ProcessName);
		return 0;
	}
}



// Resume an existing PCB
int Resume_PCB() {

	// Initialize variables
	char Process_Name[15];
	int name_size=15, error;
	PCB *TempPCB;

	if (cop!=NULL)
	{
		Insert_PCB(cop,&Blocked_queue,FIFOQ);
	}

	// Get name of process
	printf("Enter the process name (Maximum of 14 characters): ");
	fflush(stdin);
	error = sys_req(READ, TERMINAL, Process_Name, &name_size);
	if (error < 0) {
		printf("error reading input, please make sure no more than 14 characters\n");
		return 0;
	}
	string_upper(&Process_Name);
	strtok(Process_Name,"\n");


	// Look in the Ready_queue
	TempPCB = Find_PCB(&Ready_queue, Process_Name);
	if (TempPCB != NULL) {
		if (TempPCB->Process_State[1] == NOT_SUSPENDED) {
			printf("Process '%s' already ready\n",Process_Name);
		}
		else {
			TempPCB->Process_State[1] = NOT_SUSPENDED;
			error = Remove_PCB(&Ready_queue, Process_Name);
			if (error != 0) {
				printf("error removing PCB %s from ready-suspended queue",Process_Name);
			}
			error = Insert_PCB(TempPCB, &Ready_queue, PRIORITYQ);
			if (error != 0) {
				printf("error inserting PCB %s into ready-suspended queue\n",Process_Name);
			}
			printf("Process '%s' ready sucessfully\n",Process_Name);
		}
		return 0;
	}

	// Look in the Blocked_queue
	TempPCB = Find_PCB(&Blocked_queue, Process_Name);
	if (TempPCB != NULL) {
		if (TempPCB->Process_State[1] == NOT_SUSPENDED) {
			printf("Process '%s' already blocked\n",Process_Name);
		}
		else {
			TempPCB->Process_State[1] = NOT_SUSPENDED;
			printf("Process '%s' blocked sucessfully\n",Process_Name);
		}
		return 0;
	}

	// Look in the Ready_susp_queue
	TempPCB = Find_PCB(&Ready_susp_queue, Process_Name);
	if (TempPCB != NULL) {
		if (TempPCB->Process_State[1] == NOT_SUSPENDED) {
			printf("Process '%s' already ready\n",Process_Name);
		}
		else {
			TempPCB->Process_State[1] = NOT_SUSPENDED;
			error = Remove_PCB(&Ready_susp_queue, Process_Name);
			if (error != 0) {
				printf("error removing PCB %s from ready-suspended queue",Process_Name);
			}
			error = Insert_PCB(TempPCB, &Ready_queue, PRIORITYQ);
			if (error != 0) {
				printf("error inserting PCB %s into ready-suspended queue\n",Process_Name);
			}
			printf("Process '%s' ready sucessfully\n",Process_Name);
		}
		return 0;
	}


	else {
		printf("Process '%s' does not exist\n",Process_Name);
		return 0;
	}
}


// Set priority of an existing PCB
int SetPriority_PCB() {

	// Initialize variables
	char ProcessName[15];
	int name_size=15, error, flag=0, PPriority;
	PCB *TempPCB;

	// Get name of process
	printf("Enter the process name (Maximum of 14 characters): ");
	fflush(stdin);
	error = sys_req(READ, TERMINAL, ProcessName, &name_size);
	if (error < 0) {
		printf("error reading input, please make sure no more than 14 characters\n");;
		return 0;
	}
	strtok(ProcessName,"\n");

	// Get the process priority
	while (flag == 0) {
		printf("Enter the new process priority (Between -128 and +127): ");
		fflush(stdin);
		scanf("%d",&PPriority);

		// Check the priority
		if (PPriority < (-128) || PPriority > 127) {
			printf("Invalid process priority\n");
		}
		else {
			flag = 1;
		}
	}

	// Find the process
	// Look in the Blocked_queue
	TempPCB = Find_PCB(&Blocked_queue, ProcessName);
	if (TempPCB != NULL) {
		TempPCB->Process_Priority = PPriority;

		printf("Priority of process '%s' changed sucessfully\n",ProcessName);
		return 0;
	}

	// Look in the Ready_queue
	TempPCB = Find_PCB(&Ready_queue, ProcessName);
	if (TempPCB != NULL) {
		TempPCB->Process_Priority = PPriority;
		Insert_PCB(TempPCB,&Blocked_queue,FIFOQ);
		Remove_PCB(&Ready_queue,ProcessName);
		TempPCB = Find_PCB(&Blocked_queue,ProcessName);
		Insert_PCB(TempPCB,&Ready_queue,PRIORITYQ);
		Remove_PCB(&Blocked_queue,ProcessName);
		printf("Priority of process '%s' changed sucessfully\n",ProcessName);
		return 0;
	}

	// Look in the Ready_susp_queue
	TempPCB = Find_PCB(&Ready_susp_queue, ProcessName);
	if (TempPCB != NULL) {
		TempPCB->Process_Priority = PPriority;
		printf("Priority of process '%s' changed sucessfully\n",ProcessName);
		return 0;
	}
	else {
		printf("Process '%s' does not exist\n",ProcessName);
		return 0;
	}
}



//Show commands



int Show_PCB(void)
{
	/* Default Return_Value is 0 */
	int Return_Value=0;

	/* Define a pointer to a Process Control Block structure */
	PCB *ptrPCB;

	/* Define a variable to indicate whether a process was found */
	int Process_Found=1;

	/* Define variables for the process name */
	char Process_Name[15];
	int Size_Process_Name=17;

	/* Get the process name from the user */
	/* Display the prompt for the process */
	printf("\nPlease enter the process name(max 14 characters): ");

	/* Obtain the input for the new value of month */
	sys_req(READ,TERMINAL,Process_Name,&Size_Process_Name);

	/* Obtain the Process Control Block to display using Find_PCB */
	/* Check the ready queue */
	ptrPCB=Find_PCB(&Ready_queue,Process_Name);
	if(ptrPCB==NULL)
	{
		/* Nothing found, check the blocked queue */
		ptrPCB=Find_PCB(&Blocked_queue,Process_Name);
		if(ptrPCB==NULL)
		{
			/* Nothing found, check the ready suspended queue */
			ptrPCB=Find_PCB(&Ready_susp_queue,Process_Name);
			if(ptrPCB==NULL)
			{
				/* No process was found, so set to 0 */
				Process_Found=0;
			}
		}
	}

	Clear_Screen();

	if(Process_Found==0)
	{
		printf("No process was found with that name.\n");
	}
	else
	{
		Display_Process_Info(ptrPCB);
	}

	return Return_Value;
}

int Show_All(void)
{
	/* Default Return_Value is 0 */
	int Return_Value=0;

	/* Display the ready queue */
	Show_Ready();

	/* Display the blocked queue */
	Show_Blocked();

	return Return_Value;
}

int Show_Ready(void)
{
	/* Default Return_Value is 0 */
	int Return_Value=0;
	Process_List *ptrProcess_List;

	/* Define a pointer to a Process List structure */
	/* Set the pointer to the first item in the ready queue */


	ptrProcess_List = Ready_queue.Start;

	if (ptrProcess_List == NULL){
	printf("NO PCB's in Ready Queue\n");
	Pause();
	}

	while(ptrProcess_List!=NULL)
	{
		/* Display one PCB at a time */
		Clear_Screen();
		printf("*** READY QUEUE ***\n\n");
		Display_Process_Info(ptrProcess_List->Item);
		Pause();

		/* Go to the next process in the process list */
		ptrProcess_List=ptrProcess_List->Next;
	}

	/* Set the pointer to the first item in the ready queue */
	ptrProcess_List=Ready_susp_queue.Start;

	if (ptrProcess_List == NULL){
	printf("NO PCB's in Ready Suspended Queue\n");
	Pause();
	}

	while(ptrProcess_List!=NULL)
	{
		/* Display one item(PCB) at a time */
		Clear_Screen();
		printf("*** READY SUSPENDED QUEUE ***\n\n");
		Display_Process_Info(ptrProcess_List->Item);
		Pause();

		/* Go to the next process in the process list */
		ptrProcess_List=ptrProcess_List->Next;
	}

	return Return_Value;
}

int Show_Blocked(void)
{
	/* Default Return_Value is 0 */
	int Return_Value=0;

	/* Define a pointer to a Process List structure */
	/* Set the pointer to the first item in the ready queue */
	Process_List *ptrProcess_List=Blocked_queue.Start;

	if (ptrProcess_List == NULL){
	printf("NO PCB's in Blocked Queue\n");
	}
	while(ptrProcess_List!=NULL)
	{
		/* Display one item(PCB) at a time */
		Clear_Screen();
		printf("*** BLOCKED QUEUE ***\n\n");
		Display_Process_Info(ptrProcess_List->Item);
		Pause();

		/* Go to the next process in the process list */
		ptrProcess_List=ptrProcess_List->Next;
	}
	return Return_Value;
}


int Display_Process_Info(PCB *ptrPCB)
{
	/* Default Return_Value is 0 */
	int Return_Value=0;

	/* Simply display the info of the process passed in. */

	printf("PROCESS INFORMATION:\n\n");
	printf("\nName:...........%s",ptrPCB->Process_Name);
	printf("\nClass:..........%i",ptrPCB->Process_Class);
	printf("\nPriority:.......%i",ptrPCB->Process_Priority);
	if (ptrPCB->Process_State[0] == BLOCKED){
		printf("\nState:..........Blocked/");
		}
	if (ptrPCB->Process_State[0] == READY){
		printf("\nState:..........Ready/");
		}
	if (ptrPCB->Process_State[1] == SUSPENDED){
		printf("Supended");
		}
	if (ptrPCB->Process_State[1] == NOT_SUSPENDED){
		printf("Not Suspended");
		}

	printf("\nStack Size:.....%i",ptrPCB->Process_Stack_Size);
	printf("\nStack Top:......%s",ptrPCB->Process_Stack_Top);
	printf("\nMemory Size:....%i",ptrPCB->Process_Mem_Size);
	printf("\nLoad Address:...%i",ptrPCB->Load_Address);
	printf("\nExec Address:...%i",ptrPCB->Exec_Address);
	printf("\n");

	return Return_Value;
}


void Clear_Screen(void)
{
   trm_clear();
   return;
}

void Pause(void)
{
   /* Simply prompt for an input where the input is never used. */
   printf("\n");
   printf("Press Enter to continue.\n\n");
   getch();

   return;
}


void dispatch_init(void) {

	// Initialize variables
	int error;

	// Setup the interrupt vector (60h)
	error = sys_set_vec(sys_call);

	if (error != OK) {
	 printf("sys_set_vec not working correctly\n");
		}

	// Call the dispatcher
	dispatch();

	// Display exit message
	printf("\n\tDispatch is now completed.\n");
}

// Process dispatcher

void interrupt dispatch(void)
{

	static Process_List *proc_list;
	static int ss_simple, sp_simple;
	proc_list = Ready_queue.Start;
	// Initialize new stack
	if (sp_save == NULL) {
		// Save old stack addresses and create a new stack
		ss_save = _SS;
		sp_save = _SP;

	}

	// Check for an empty queue
	if (proc_list == NULL) {
		cop = NULL;
		_SS = ss_save;
		_SP = sp_save;
		ss_save = NULL;
		sp_save = NULL;
	}

	// Dispatch process on the ready queue
	else {
		cop = Find_PCB(&Ready_queue,proc_list->Item->Process_Name);

		Remove_PCB(&Ready_queue,cop->Process_Name);

		cop->Process_State[0] = RUNNING;

		ss_simple = FP_SEG(cop->Process_Stack_Top);
		sp_simple = FP_OFF(cop->Process_Stack_Top);
		_SS=ss_simple;
		_SP=sp_simple;
	}



}

void interrupt sys_call(void)

{
  // Initialize variables
  static int error;

  trm_getc(); /* copy chars from DOS buffer to MPX buffer */

  /* determine if any event flags are set */

  if (print_control_block.eflag == 1)
    io_completion(PRINTER);

  if (com_control_block.eflag == 1) {
    if (com_control_block.op_code == READ) {
	(*com_control_block.count_p) += 2;
	strcat(com_control_block.buf_p,"\r\n");

    }
    io_completion(COM_PORT);
  }

  if (term_control_block.eflag == 1)
    io_completion(TERMINAL);

	// Get new stack pointer
	cop->Process_Stack_Top = (unsigned char *) MK_FP(_SS, _SP);
	cop->context_p=(context *)cop->Process_Stack_Top;
	// Get the op_code
	param_p=(parameters*)((char *)MK_FP(_SS,_SP)+sizeof(context));
	// Check the op_code
	switch (param_p->op_code) {

		// IDLE case
		case IDLE:
			disable();
			cop->Process_State[0] = READY;
			error = Insert_PCB(cop, &Ready_queue, PRIORITYQ);
			if (error != 0) {

			}
			context_temp->AX = 0;
			enable();
			break;

		// EXIT case
		case EXIT:
			printf("Termination request for '%s' detected.\n",cop->Process_Name);
			context_temp->AX = 0;
			Free_PCB(cop);
			break;


    case READ:
    case WRITE:
    case CLEAR:

      io_scheduler(cop, param_p->op_code,  param_p->device_id,
			param_p->buf_p, param_p->count_p);

      break;



    default: /* should never reach here */

      // context_p->AX= -1000;

    break;

    }

  dispatch();

}

void create_stack(struct PCB *ptr)
{
	ptr -> Process_Stack_Top = (unsigned char *)(ptr-> Process_Stack + 1024 - sizeof(context));
	ptr -> context_p = (context*)(ptr-> Process_Stack_Top);
	ptr->context_p->DS = _DS;
	ptr->context_p->ES = _ES;
	ptr->context_p->FLAGS = 0x200;
	ptr->context_p->CS = FP_SEG(ptr->Exec_Address);
	ptr->context_p->IP = FP_OFF(ptr->Exec_Address);
}

int load ()
{

	int count, count2, pri, error;
	struct PCB* v_pcb;
	char p_name[15],pr[20];
	int Start_Offset ;
	int Program_Length,name_size=15;
	char* Name;
	void *L_Add;

	printf("Enter the program name (Maximum of 14 characters):\n ");
	printf("please do not include extension (ex. proc1):");
	fflush(stdin);
	error = sys_req(READ, TERMINAL, p_name, &name_size);
	if (error < 0) {
		printf("unable to read input, please only 14 characters or less\n");
		return 0;
	}
	strtok(p_name,"\n");
	string_upper(p_name);
	Create_PCB(p_name);



	v_pcb = Find_PCB(&Ready_queue, p_name);
	error = sys_check_program("c:\\cs256\\mpx\\", p_name, &Program_Length, &Start_Offset);
	if (error<0)
	{
	printf("error in sys check, %i\n, if -117 file not found",error);
	return 1;
	}

	L_Add = sys_alloc_mem((size_t) Program_Length);
	v_pcb -> Exec_Address = MK_FP(L_Add, Start_Offset);

	v_pcb -> Load_Address = L_Add;

	create_stack(v_pcb);
	error = sys_load_program(v_pcb -> Load_Address, Program_Length, "c:\\cs256\\mpx\\", p_name);
	if (error <0)
	{
	printf("load error %i,\n",error);
	return error;
	}
	//suspend PCB to be resumed by the user using the Resume command
	Suspend_PCB (p_name);
 return 0;
}

int terminate()
{
  int name_size=15,error;
  struct PCB* TempPCB;
  char *ProcessName;


	printf("Enter the process name to be terminated(Maximum of 14 characters): ");
	fflush(stdin);
	error = sys_req(READ, TERMINAL, ProcessName, &name_size);
	if (error < 0) {
		printf("unable to read input, please only 14 characters or less\n");
		return 0;
	}
	strtok(ProcessName,"\n");
	string_upper(ProcessName);
  TempPCB = Find_PCB(&Ready_queue, ProcessName);

  if (TempPCB->Process_Class == SYSTEM_P) {
  printf("cannot terminate system processes\n");
  return 1;
  }
  if (TempPCB != NULL) {

		error = Remove_PCB(&Ready_queue, ProcessName);
		if (error < 0) {
			printf("error removing PCB %s from ready queue\n",ProcessName);
			return 0;
		}

		error=sys_free_mem(TempPCB->Load_Address);
		if(error<0){
			printf("error freeing program memory\n");
			}
		Free_PCB(TempPCB);
		printf("Process '%s' terminated sucessfully\n",ProcessName);
		return 0;
	}

	// Look in the Blocked_queue




	TempPCB = Find_PCB(&Blocked_queue, ProcessName);

	if (TempPCB != NULL) {

		error = Remove_PCB(&Blocked_queue, ProcessName);
		if (error < 0) {
			printf("error removing PCB %s from blocked queue.\n",ProcessName);
			return 0;
		}
		error=sys_free_mem(TempPCB->Load_Address);
		if(error<0){
			printf("error freeing program memory\n");
			}
		Free_PCB(TempPCB);
		printf("Process '%s' terminated sucessfully\n",ProcessName);
		return 0;
	}

	// Look in the Ready_susp_queue

	TempPCB = Find_PCB(&Ready_susp_queue, ProcessName);
	if (TempPCB != NULL) {
		error = Remove_PCB(&Ready_susp_queue, ProcessName);
		if (error < 0) {
			printf("Error removing PCB %s from Ready_susp_queue\n",ProcessName);
			return 0;
		}
		error=sys_free_mem(TempPCB->Load_Address);
		if(error<0){
			printf("error freeing program memory\n");
			}
		Free_PCB(TempPCB);
		printf("Process '%s' terminated sucessfully\n",ProcessName);
		return 0;
	}
	else {
		printf("Process '%s' does not exist\n",ProcessName);
		return 0;
	}

  }

int load_idle ()
{

	int count, count2, pri, error;
	struct PCB* v_pcb;
	char p_name[15],pr[20];
	int Start_Offset ;
	int Program_Length;
	char* Name;
	void *L_Add;

	pri = -128;
	Create_known_pcb("IDLE",pri);



	v_pcb = Find_PCB(&Ready_queue, "IDLE");
	error = sys_check_program("c:\\cs256\\mpx\\", "idle", &Program_Length, &Start_Offset);
	if (error<0)
	{
	printf("error in sys check, %i\n, if -117 file not found",error);
	return 1;
	}

	L_Add = sys_alloc_mem((size_t) Program_Length);
	v_pcb -> Exec_Address = MK_FP(L_Add, Start_Offset);

	v_pcb -> Load_Address = L_Add;

	create_stack(v_pcb);
	error = sys_load_program(v_pcb -> Load_Address, Program_Length, "c:\\cs256\\mpx\\", "idle");
	if (error <0)
	{
	printf("load error %i,\n",error);
	return error;
	}

 return 0;
}

int load_comhan()
{
	//initialize variables
	PCB *TempPCB;
	context *context_p;
	int PState[2], *test,*test_5;
	char ProcessName[6][15];
	int i;
	// insert test into ready queue


		TempPCB = Allocate_PCB();
		PState[0]=READY;
		PState[1]=NOT_SUSPENDED;
		if (TempPCB == NULL) {
			printf("Unable to create new PCB, number of PCB's reached max\n");
			return 0;
		}
		// debug
		Setup_PCB(TempPCB,"comhand",SYSTEM_P,127,PState);

		// Setup the process stack pointer

		TempPCB->Process_Stack_Top =TempPCB->Process_Stack+sizeof(TempPCB->Process_Stack)
		- sizeof(context);

		// Initialize the 'context' area of the stack

		TempPCB->context_p=(context *)TempPCB->Process_Stack_Top;
		TempPCB->context_p->DS=_DS;
		TempPCB->context_p->ES=_ES;
		TempPCB->context_p->CS=FP_SEG(&comhan);
		TempPCB->context_p->IP=FP_OFF(&comhan);
		TempPCB->context_p->FLAGS=0x200;

		//TempPCB->Exec_Address = (unsigned int*)test_func_R3[i];
		Insert_PCB(TempPCB,&Ready_queue,PRIORITYQ);
		// # 4

		return 0;
}

PCB * dequeue(Header *queue)

{

  PCB *ret = NULL; //pointer to be returned

  Process_List *qnode;



  qnode=queue->Start;



  if (qnode != NULL) {

    queue->Start=qnode->Next;

    if (queue->Start != NULL)

      queue->Start->Previous = NULL;



    ret=qnode->Item;

    sys_free_mem(qnode);

  }

  return ret;

}
