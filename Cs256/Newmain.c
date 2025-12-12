/***********************************************************************

  MPX: The MultiProgramming eXecutive



  File Name: newmain.c



  Author:Wes Queen

  Version: 6.0

  Date: 04/15/01



  Purpose: Main function for MPX-PC



  Functions:

    main






************************************************************************/



#include <stdio.h>
#include "mpx_supt.h"
#include "comhand.h"
#include "pcb_cont.h"
#include "dos.h"
#include "trmdrive.h"
#include "drivers.h"
#include "ioschd.h"


/*  Procedure: main
 *
 *  Purpose: initialize mpx and call the command handler.
 *
 *  Parameters: none
 *
 *  Return value: a value of 0 returned to the OS
 *          from within sys_exit
 *
 *  Calls: sys_init
 *         sys_set_vec
 *         alloc_pcb
 *         setup_pcb
 *         MK_FP
 *         FP_SEG
 *         FP_OFF
 *         insert
 *         sys_alloc_mem
 *         sys_check_program
 *         sys_load_program
 *         printf
 *         dispatch
 *         sys_exit
 *
 *  Globals: ready
 *
 *  Errors: none
 *
 *  Summary of Algorithm:
 *     Initialize the system, print a welcome message, call
 *     the command handler, print a closing message, and
 *     clean up the system.
 */
int main(void)
{
  extern Header Ready_queue;
  int err;
  struct PCB *pcb;
  context *context_p;
  int Program_Length, offset,error,Start_Offset;
  int PState[2];
  extern IOCB print_control_block, com_control_block, term_control_block;
  void *L_Add;


  sys_init(MODULE_F); /* initialize MPX OS */
  sys_set_vec(sys_call); /* link int60h interrupt */


  /* initialize IO control blocks */
  init_cb(&print_control_block);
  init_cb(&com_control_block);
  init_cb(&term_control_block);

  /* open device drivers */

  prt_open((int far*) &print_control_block.eflag);
  com_open((int far*) &com_control_block.eflag, 1200);
  trm_open((int far*) &term_control_block.eflag);


  /* install command handler as a process */

  /* MPX OS welcome message */
    load_comhan();
    load_idle();
	fflush(stdin);
	printf("Welcome to COMHAN MPX\n");
	printf("Please enter a command or type 'help list' to begin\n");
	mpx_date("");
       //	comhan();


  dispatch(); /* start running processes */



  /* close device drivers */

  prt_close();

  com_close();

  trm_close();


  /* MPX OS closing message */

  printf("%s%s%s\n%s\n\n",

   "Thanks for using MPX Final Module.",

   "You have exited the system.");



  sys_exit(); /* cleanup MPX system and exit */



  return 0;

}

