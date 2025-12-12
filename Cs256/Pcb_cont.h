// Define values
#define SYSTEM_P 0
#define APPLICATION_P 1
#define RUNNING 0
#define READY 1
#define BLOCKED 2
#define NOT_SUSPENDED 0
#define SUSPENDED 1
#define PRIORITYQ 0
#define FIFOQ 1

// Declaration of the context of registers for saving on the stack
typedef struct context {
	unsigned int BP, DI, SI, DS, ES, DX, CX, BX, AX, IP, CS, FLAGS;
}context;

// Define the structure that contains the information about each PCB
typedef struct PCB {
	char Process_Name[15];      // Process name
	int Process_Class;         // Process class
	int Process_Priority;      // Process priority
	int Process_State[2];      // Process state
	int Process_Stack_Size;    // Process stack size
	unsigned char * Process_Stack_Top;   // Process stack pointer to top of stack
	int Process_Mem_Size;      // Process memory size
	unsigned int * Load_Address;	// Process memory load address
	unsigned int * Exec_Address;  // Process memory execution address
	unsigned char Process_Stack[4096];  // Process stack
	context *context_p;
}PCB;

// Declaration of a linked list for type PCB
typedef struct Process_List {
	PCB *Item;        	    // Each item will be of type PCB
	struct Process_List *Next;     // Point to next Process_List object
	struct Process_List *Previous; // Point to the previous Process_List object
}Process_List;



// Declaration of the Start and End
typedef struct Header {
	Process_List *Start;	// Pointer to the start of a queue
	Process_List *End;   // Pointer to the end of a queue
}Header;


// Function to allocate memory for a PCB
PCB * Allocate_PCB();

// Function to setup the values for a PCB
int Setup_PCB(PCB *, char[15], int, int, int[2]);

//function to insert PCB into queue
int Insert_PCB(PCB *, Header *, int);

//function to remove PCB from queue
int Remove_PCB(Header *, char[15]);

// Function to deallocate memory for a PCB
void Free_PCB(PCB *);


// Temporary functions for R2

// Function to create processes
int Create_PCB(char[15]);

// Function to delete processes
int Delete_PCB();

// Function to block processes
int Block_PCB();

// Function to unblock processes
int Unblock_PCB();


// Function to suspend processes
int Suspend_PCB(char[15]);

// Function to unsuspend processes
int Resume_PCB();

void create_stack(struct PCB *);
//show commands
/* Define prototypes */
int Display_Process_Info(PCB*);
int Show_Ready(void);
int Show_Blocked(void);
void Clear_Screen(void);
void Pause(void);
void interrupt dispatch(void);
int load();
void dispatch_init(void);
void interrupt sys_call(void);
int terminate();
typedef struct parameters {
	int      op_code;
	int      device_id;
	char     *buf_p;
	int      *count_p;
	} parameters;

PCB * dequeue(Header *queue);


void test1_R3(void);
void test2_R3(void);
void test3_R3(void);
void test4_R3(void);
void test5_R3(void);

