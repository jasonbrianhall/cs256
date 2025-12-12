
/* Created 01/28/01 by Wes Queen			      */
/* This contains procedures exit_to_system,comhan, and main   */
/* It will provide the CLI(command line interface) to the user */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include "pcb_cont.h"
#include "ioschd.h"
#include "mpx_supt.h"




void comhan(void)
{

  int exit_match, length;
  int end_comhnd,i,comsz,find,cmnd_num;
  char cmndin[40]="\0";
  char *args[2];
  char cmnd[28][15];
  char blank_string[20],arguement1[20];
  int check;
  int num_of_cmnds = 28;
  PCB *pcb;
 /* stack=create_stack();
  initialize_stack(stack);*/
 IO_REQ *ioreq;
 extern Header Blocked_queue,Ready_queue,Ready_susp_queue,Blocked_susp_queue;
 extern IOCB term_control_block;
 extern IOCB com_control_block;
 extern IOCB print_control_block;


  /* the following will define the commands to be checked against the users input */

  strcpy(cmnd[1],"VER");
  strcpy(cmnd[2],"VERSION");
  strcpy(cmnd[3],"VR");
  strcpy(cmnd[4],"DATE");
  strcpy(cmnd[5],"DT");
  strcpy(cmnd[6],"DIRECTORY");
  strcpy(cmnd[7],"DIR");
  strcpy(cmnd[8],"LS");
  strcpy(cmnd[9],"HELP");
  strcpy(cmnd[10],"EXIT");
  strcpy(cmnd[11],"SYSTEM");
  strcpy(cmnd[12],"CD");
  strcpy(cmnd[13],"CREATE_PCB");
  strcpy(cmnd[14],"DELETE_PCB");
  strcpy(cmnd[15],"BLOCK");
  strcpy(cmnd[16],"UNBLOCK");
  strcpy(cmnd[17],"SUSPEND");
  strcpy(cmnd[18],"RESUME");
  strcpy(cmnd[19],"SET_PRIORITY");
  strcpy(cmnd[20],"SHOW_PCB");
  strcpy(cmnd[21],"SHOW_ALL");
  strcpy(cmnd[22],"SHOW_READY");
  strcpy(cmnd[23],"SHOW_BLOCKED");
  strcpy(cmnd[24],"LOAD");
  strcpy(cmnd[25],"DISPATCH");
  strcpy(cmnd[26],"TERMINATE");
  strcpy(cmnd[27],"HISTORY");

  end_comhnd = 1000;
  exit_match = 0;

	restart_entry_handler:   /* this will be jumped to if user enters carrage return */

  /* the following will loop until the user enters "exit" or "system" */
  comsz=1000;
  while (end_comhnd != exit_match)	        /* run loop while not exit statement */
  {
	strcpy(cmndin,"               ");;
	comsz=1000;
	fflush(stdin);
	printf("\t");
	printf("MPX:>");		        /* print prompt	*/
	sys_req(READ,TERMINAL,cmndin,&comsz);	/* get command from user */
	args[0] = strtok(cmndin," ");

	if (strcmp(args[0],"\0") != 0)		/* carrage return, start over*/
	{
	/* now check for arguements  */

	args[1] = strtok(NULL," ");


	/* check to see if no arguement and if none then set arg[1] to space */

	if (args[1] == NULL)
	{
	check=0;
	for(i=0;i<=strlen(args[0]); i++)
	{
	strcpy(blank_string, "");
	strncat(blank_string, args[0]+i, 1);
	if(strcmp(blank_string, " ")==0 & check==0);
	{
	check=1;
	length=0;
	length=i;

	}
	}
	strcpy(blank_string, "");
	strncat(blank_string,args[0], length-1);
	cmnd_num = 1000;
	}
	else{
	strcpy(blank_string,"");
	strcpy(blank_string,args[0]);

	cmnd_num=1000;
	}

	if (args[1] != NULL)
	{
	check=0;
	for(i=0;i<=strlen(args[1]); i++)
	{
	strcpy(arguement1, "");
	strncat(arguement1, args[1]+i, 1);
	if(strcmp(arguement1, " ")==0 & check==0);
	{
	check=1;
	length=0;
	length=i;

	}
	}
	strcpy(arguement1, "");
	strncat(arguement1,args[1], length-1);
	cmnd_num = 1000;
	strcpy(args[1], "");
	strcpy(args[1], arguement1);
	}


	/* now copy the blank string containing the command back into */
	/* args[0]						      */
	strcpy(args[0],"");
	strcpy(args[0],blank_string);
	strcpy(blank_string,"");
	string_upper(args[0]);
	string_upper(args[1]);

	/* now compare the users input to the commands stored */
	for (find=1;find<num_of_cmnds;find++)
	{
		if (strcmp(args[0],cmnd[find]) == 0)
		{
			 cmnd_num = find;
			 break;
		}
	}



	/* the following checks to see if the value of cmnd_num has changed since trying */
	/* to compare to user input.  If the value has not changed that means the input  */
	/* by the user is not valid and the user needs to reenter the command		 */


	if ((cmnd_num == 1000) & (strcmp(args[0],"\0") != 0))
	{
	printf("Command '%s' is not found, please reenter your command or type 'help list'\n",args[0]);
	goto restart_entry_handler;
	}

	/* the following will be used for debug to see if the input by the user has  */
	/* been received correctly.  Will be deleted                                 */


	/* call the command according to the value given by cmnd_num  */

	switch(cmnd_num) {

	case 1: case 2: case 3:
		{
		version();
		break;
		}
	case 4: case 5:
		{

		mpx_date(args[1]);
		break;
		}
	case 6: case 7: case 8:
		{
		list();
		break;
		}
	case 9:
		{

		  help(args[1]);
		  break;
		}
	case 10: case 11:
		{

		end_comhnd = 0;
		break;
		}
	case 12:
		{
		cd(args[1]);
		break;
		}
	case 13:
		{
		Create_PCB(args[1]);
		break;
		}
	case 14:
		{
		Delete_PCB();
		break;
		}
	case 15:
		{
		Block_PCB();
		break;
		}
	case 16:
		{
		Unblock_PCB();
		break;
		}
	case 17:
		{
		Suspend_PCB(args[1]);
		break;
		}
	case 18:
		{
		Resume_PCB();
		break;
		}
	case 19:
		{
		SetPriority_PCB();
		break;
		}
	case 20:
		{
		Show_PCB();
		break;
		}
	case 21:
		{
		Show_All();
		break;
		}
	case 22:
		{
		Show_Ready();
		break;
		}
	case 23:
		{
		Show_Blocked();
		break;
		}
	case 24:
		{
		load();
		break;
		}
	case 25:
		{
		dispatch_init();
		break;
		}
	case 26:
		{
		terminate();
		break;
		}

	case 27:
		{
	       //hist(atoi(args[1]), stack);
		break;
		}
	}
  }  /* end carrage return if statment */


  } /* end while loop  */

   disable();

 /* perform clean up. Make sure all of the pcb's loaded programs
    have been cleaned up, and then delete the pcb */

 pcb = dequeue(&Ready_susp_queue); //

  while (pcb != NULL) {

    sys_free_mem(pcb->Load_Address);

    free_pcb(pcb); /* free the memory allocated for pcb */

    pcb = dequeue(&Ready_susp_queue);

  }

  pcb = dequeue(&Blocked_susp_queue);

  while (pcb != NULL) {

    sys_free_mem(pcb->Load_Address);

    free_pcb(pcb); /* free the memory allocated for pcb */

    pcb = dequeue(&Blocked_susp_queue);

  }

  pcb = dequeue(&Blocked_queue);

  while (pcb != NULL) {

    sys_free_mem(pcb->Load_Address);

    free_pcb(pcb); /* free the memory allocated for pcb */

    pcb = dequeue(&Blocked_queue);

  }

  pcb = dequeue(&Ready_queue);

  while (pcb != NULL) {

    sys_free_mem(pcb->Load_Address);

    free_pcb(pcb); /* free the memory allocated for pcb */

    pcb = dequeue(&Ready_queue);

  }



  ioreq = io_dequeue(&term_control_block.q);

  while (ioreq != NULL) {

    sys_free_mem(ioreq);

    ioreq = io_dequeue(&term_control_block.q);

    }

  ioreq = io_dequeue(&com_control_block.q);

  while (ioreq != NULL) {

    sys_free_mem(ioreq);

    ioreq = io_dequeue(&com_control_block.q);

    }

  ioreq = io_dequeue(&print_control_block.q);

  while (ioreq != NULL) {

    sys_free_mem(ioreq);

    ioreq = io_dequeue(&print_control_block.q);

    }





  term_control_block.car=NULL;

  print_control_block.car=NULL;

  com_control_block.car=NULL;



  enable();



 sys_req(EXIT,TERMINAL,cmndin,&comsz);
}  /* end comhan  */

