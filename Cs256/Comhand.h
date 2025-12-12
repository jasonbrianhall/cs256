/***********************************************************************

  MPX Command Handler



  File Name: comhan.h


  Purpose: Header file for Command Handler



    This file contains command handler declarations

    intended to receive a command and execute it.



  Functions:



    com_han



************************************************************************/



#ifndef _COMHAND_H

#define _COMHAND_H



/*  Procedure: comhand

 *

 *  Purpose: read in a command from the user and send it

 *           to command_run in an acceptable form

 *

 *  Parameters: none

 *

 *  Return value: none

 *

 *  Errors: If invalid command is entered, a message is

 *          displayed to the screen stating that.

 */

void comhan(void);


#endif