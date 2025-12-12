
/***********************************************************************



  Printer Driver functions



  File Name:  drivers.c



  Author:  Wes Queen,Kenny Heck

  Version: 5.0

  Date:    04/10/01



  Purpose: device drivers for printer and serial port



  Functions:  prt_open

	      prt_close

	      prt_write

	      prt_call

	      com_open

	      com_close

	      com_read

	      com_write

	      com_handler

	      com_input

	      com_output



************************************************************************/

#include "drivers.h"
#include <stdio.h>
#include <dos.h>
#include <conio.h>

int IRQ=7;               /* IRQ here */


/* old interrupt handlers - saved and restored on exit */
void interrupt (*curr_com_address)(void);
void interrupt (*old_prt_handler)(void);

/* internal function prototypes */
void interrupt prt_call(void);
void interrupt com_handler(void);
void com_input(void);
void com_output(void);

unsigned char result; /* used in serial port interrupt handler */

DCB printer = {UNUSED, 0};
DCB com;


/********************
 ** Printer Driver **
 ********************/

int picaddr; /* PIC Base Address */
int intno;   /* Interrupt Vector Number */
int picmask; /* PIC's Mask */

/*  Procedure: prt_open
 *
 *  Purpose: initialization of the printer
 *
 *  Parameters: eflag_p --a pointer to an integer event flag. This will
 *                        be used to indicate when a requested data
 *                        transfer has been completed
 *
 *  Return value: 0    if okay
 *                -101 (PRT_INV_EFLAG) if invalid event flag pointer
 *                -102 (PRT_ALR_OPEN) if printer is already open
 *
 *  Globals: printer
 *           old_prt_handler
 *           picaddr
 *           intno
 *           picmask
 *
 *  Calls: getvect
 *         setvect
 *         inportb
 *         outportb
 *         delay
 *
 *  Errors: none
 *
 *  Summary of Algorithm:
 *     First, the parameter and the printer state are checked. Then, if
 *     everything is fine, it initializes the DCB, switches printer
 *     interrupt handlers, enables them for the device, and then finally
 *     initializes the printer.
 */
int prt_open (int *eflag_p)
{
 int i;
  /* ensure the event flag pointer is valid */
  if (eflag_p == NULL) {
		return PRT_INV_EFLAG;
  }
  /* ensure the printer is not currently open */
  if (printer.ready_flag == OPEN) {
		return PRT_ALR_OPEN;
  }

  /* initialize the printer DCB */
  printer.ready_flag = OPEN;
  printer.status = IDLE;
  printer.eflag_p = eflag_p;

  /* Calculate Interrupt Vector, PIC Addr & Mask */
  if (IRQ >= 2 && IRQ <= 7) {
					intno = IRQ + 0x08;
					picaddr = PIC1;
					picmask = 1;
					picmask = picmask << IRQ;
				 }
  if (IRQ >= 8 && IRQ <= 15) {
					 intno = IRQ + 0x68;
										 picaddr = PIC2;
										 picmask = 1;
										 picmask = picmask << (IRQ-8);
				  }
  if (IRQ < 2 || IRQ > 15) {
		printf("IRQ Out of Range\n");
		return 1;
  }

  old_prt_handler = getvect(intno); /* Save old Interrupt Handler */
  setvect(intno, &prt_call);   /* Set new Interrupt Vector Entry */

  /* Enable printer interrupts by un-masking PIC */
  outportb(picaddr+1, inportb(picaddr+1) & (0xFF - picmask));

  outportb(CONTROL, 0x08); /* set initialize bit */
  for (i=0; i<5000; i++);
  outportb(CONTROL, 0x0C); /* set select bit */

  return 0;
}


/*  Procedure: prt_close
 *
 *  Purpose: closing the printer and restoring initial values
 *
 *  Parameters: none
 *
 *  Return value: 0    if okay
 *                -201 (PRT_NOT_OPEN) if printer not open
 *
 *  Globals: printer
 *           oldfunc
 *
 *  Calls: setvect
 *         enable
 *         disable
 *         inportb
 *         outportb
 *         delay
 *
 *  Errors: none
 *
 *  Summary of Algorithm:
 *     First, the printer state is checked. Then, if everything is fine,
 *     it clears the open indicator in the DCB, disables printer interrupts,
 *     switches printer interrupt handlers to the original, and resets the
 *     printer.
 */
int prt_close (void)
{
  /* ensure printer is currently open */
  if (printer.ready_flag != OPEN) {
      return PRT_NOT_OPEN;
  }

  /* clear the open indicator */
  printer.ready_flag = CLOSED;

  outportb(CONTROL, inportb(CONTROL) & 0xEF); /* disable Parallel Port IRQ's */
  outportb(picaddr+1, inportb(picaddr+1) | picmask); /* Mask PIC */

  setvect(intno, old_prt_handler); /* restore old Interrupt Vector */

  return 0;
}


/*  Procedure: prt_write
 *
 *  Purpose: initiates the transfer of a block of data to the printer
 *
 *  Parameters: buf_p   --starting address of the buffer containing the
 *                        block of characters to be printed
 *              count_p --address of an integer count value indicating
 *                        the number of characters to be printed. This
 *                        will be modified to show the number of chars
 *                        actually transferred.
 *
 *  Return value: 0    if no errors
 *                -401 (PRT_WR_NOT_OPEN) if printer not open
 *                -402 (PRT_INV_BUF) if invalid buffer address
 *                -403 (PRT_INV_COUNT) if invalid count address or value
 *                -404 (PRT_BUSY) if printer busy
 *
 *  Globals: printer
 *
 *  Calls: enable
 *         disable
 *         outportb
 *         
 *
 *  Errors: none
 *
 *  Summary of Algorithm:
 *     Check printer status and parameters. Set buffer and count in
 *     the DCB and reset various other values. Clear the event flag,
 *     write a null character to the data register, and forward the
 *     data to the printer.
 */
int prt_write (char *buf_p, int *count_p)
{

  int i;
  /* ensure buf_p is valid */
  if (buf_p == NULL) {
		return PRT_INV_BUF;
  }
  /* ensure count_p is valid and not negative */
  if (count_p == NULL || *count_p < 0) {
		return PRT_INV_COUNT;
  }
  /* ensure printer is open */
  if (printer.ready_flag != OPEN) {
		return PRT_WR_NOT_OPEN;
  }
  /* ensure printer is not busy */
  if (printer.status != IDLE) {
		return PRT_BUSY;
  }

  /* put buffer and count in printer DCB */
  printer.buffer = buf_p;
  printer.count_var = count_p;

  printer.status = WRITING;

  /* reset various counts */
  printer.buf_size = *count_p;
  printer.current_loc = 0;
  printer.ntransfer = 0;

  *printer.eflag_p = 0;

  /* write a null character to the data register */
  outportb(DATA, 0x00);
  for (i=0; i<20; i++);
  /* forward the data to the printer */
  outportb(CONTROL, 0x1D); /* turn on strobe and interrupts */
   for (i=0; i<20; i++);
  disable();
  outportb(CONTROL, 0x1C); /* turn off strobe */
  enable();

  return 0;
}


/*  Procedure: prt_call
 *
 *  Purpose: reset the interrupt and transfer the next character
 *
 *  Parameters: none
 *
 *  Return value: none
 *
 *  Globals: printer
 *
 *  Calls: enable
 *         disable
 *         outportb
 *         
 *
 *  Errors: none
 *
 *  Summary of Algorithm:
 *     First, send the EOI. Then, if the printer is open and writing,
 *     check if there are characters to write. If there are, put the
 *     next character in the data register and forward it to the
 *     printer. If not, set the printer status to idle, update the
 *     requestor's count varable and event flag.
 */
void interrupt prt_call(void)
{
  static int i;
  outportb(picaddr, 0x20); /* End of Interrrupt (EOI) */

  /* if printer not ready or not writing, ignore interrupt */
  if (printer.ready_flag == OPEN && printer.status == WRITING) {
    if (printer.ntransfer < printer.buf_size) {
      /* put the next character in the data register */
      outportb(DATA, printer.buffer[printer.ntransfer]);
		 for (i=0; i<20; i++);
      printer.ntransfer++;

      /* forward the data to the printer */
      outportb(CONTROL, 0x1D); /* turn on strobe and interrupts */
      for(i=0; i<20; i++);
      outportb(CONTROL, 0x1C); /* turn off strobe */

    } else { /* if no more characters to write */
      printer.status = IDLE;
      *printer.count_var = printer.ntransfer;
      *printer.eflag_p = 1;
    }
  }
}


/************************
 ** Serial Port Driver **
 ************************/

unsigned char data_buff;

/****************************************************************************
 Procedure : com_open

 Purpose : The com_open function is called to initialize the serial port.

 Parameters : eflag_p is a pointer to an integer event flag within the calling program
              baud_rate is is an integer value representing the desired baud rate

 Return Value :  0  -- No Error

 Errors :  COM_INV_EFLAG  --Invalid (null) event flag pointer
           COM_INV_BAUD   --Invalid baud rate divisor
           COM_ALR_OPEN   --Port already open

 Functions used :  setvect, getvect, disable, enable, outportb

 Summary of Algorithm :

 Initialize the DCB; Set the new interrupt handler address 	into the interrupt vector; Compute and store the baud rate divisor; Set the other necessary line characteristics; Enable all of the necessary interrupts
************************************************************************************/

int com_open(int *eflag_p, int baud_rate)
{
  unsigned int baud_rate_d;

  /*If the event flag pointer is NULL, return error code */
  if (eflag_p==NULL)
    return COM_INV_EFLAG;

  /*If baud rate does not equal 1200, return error code */
  if (baud_rate!=1200)
    return COM_INV_BAUD;

  /*Is the device currently open?*/
  if (com.ready_flag==OPEN)
    return COM_ALR_OPEN;

  /*Intialize the DCB*/
  com.ready_flag = OPEN;
  com.eflag_p = eflag_p;
  com.status = IDLE;

  /*Initialize the ring buffer*/
  com.head = 0;
  com.tail = 0;

  curr_com_address = getvect(INT_ID); /* save old interrupt handler */
  setvect(INT_ID, &com_handler); /* set to our new interrupt handler */

  /*Compute the required baud rate divisor*/
  baud_rate_d  = 115200 / (long) baud_rate;

  outportb(LINE_CONTROL,0X80); /* allow access to baud rate divisor register */

  /* store baud rate divisor in register */
  outportb(HIGH, (char) baud_rate_d >> 8);
  outportb(LOW, (char) baud_rate_d);

  /* set line characteristics to 8 data bits, 1 stop bit, and no parity */
  /* Also restore normal functioning of the first two ports */
  outportb(LINE_CONTROL,0x03);

  disable();
  outportb(PIC1+1, inportb(PIC1+1) & ~0x10); /* enable level in PIC mask register */
  enable();

  outportb(MODEM_CONTROL,0x08); /* enable serial port interrupts */
  outportb(INT_EN,0x01); /* enable input ready interrupts */

  return 0;
}


/*************************************************************************************
 Procedure : com_close

 Purpose :

 The com_close function will be called at the end of a session of serial port use.

 Parameters : None

 Return Value :  0  -- No Error

 Errors :  COM_NOT_OPEN  -- Serial port not open

 Functions used :  setvect, outportb, inportb

 Summary of Algorithm :

 Ensure that the port is currently open. Clear the open indicator in the DCB. Disable the appropriate level in the PIC mask register. Disable all interrupts in the ACC by loading zero values to the Modem Status register and the Interrupt Enable register. Restore the original saved interrupt vector.
*************************************************************************************/

int com_close(void)
{
  /*Ensure that the port is currently open*/
  if (com.ready_flag != OPEN)
    return COM_NOT_OPEN;

  com.ready_flag=CLOSED;

  /*Disable the appropriate level in the PIC mask register*/
  outportb(PIC1+1, inportb(PIC1+1) | 0x10);

  /* disable all interrupts in ACC */
  outportb(MODEM_STATUS,0);
  outportb(INT_EN,0);

  /*restore the original saved interrupt vector*/
  setvect(INT_ID, curr_com_address);

  return 0;
}


/*************************************************************************************
 Procedure : com_read

 Purpose :

 The com_read function obtains input characters and loads them into the requestor's buffer.

 Parameters :  buf_p is a far pointer to the starting address of the buffer to
	       receive the input characters

	       count_p is the address of an integer count value indicating the number of characters to be read.

 Return Value :  0  -- No Error

 Errors :  COM_RD_NOT_OPEN  -- Port not open
           COM_RD_INV_BUF  -- Invalid buffer address
	   COM_RD_INV_COUNT  -- Invalid count address or count value
	   COM_RD_BUSY  -- Device busy

 Functions used :  disable, enable

 Summary of Algorithm :

 Validate the supplied parameters. Ensure that the port is open, and the status is idle. Initialize the input buffer variables and set the status to reading. Clear the caller's event flag. Copy characters from the ring buffer to the requestor's buffer, until the ring buffer is emptied, the requested count has been reached, or a CR (ENTER) code has been found. Either input interrupts or all interrupts should be disabled during the copying. If more characters are needed, return. If the block is complete, continue. Reset the DCB status to idle, set the event flag, and return the actual count to the requestor's variable
*************************************************************************************/

int com_read(char *buf_p,int *count_p)
{
  if (com.ready_flag != OPEN)
    return COM_RD_NOT_OPEN;

  if (buf_p==NULL)
    return COM_RD_INV_BUF;

  if ((count_p==NULL) || (*count_p < 0))
    return COM_RD_INV_COUNT;

  if (com.status != IDLE)
    return COM_RD_BUSY;

  com.status=READING;

  /*Initialize the input buffer variables*/
  com.buffer = buf_p;
  com.count_var = count_p;
  com.inp_count=0;

  /*Clear the caller's event flag*/
  *com.eflag_p = 0;
  com.trav = buf_p;

  disable(); /*interrupts MUST be disabled while copying from ring buffer*/

  /* Copy characters from ring buffer to requestor's buffer until:
   *    1. the ring buffer is emptied,
   *    2. the requested count has been reached,
   * OR 3. a CR (ENTER) code has been found
   */
  while ((*com.count_var < com.inp_count) &&
         (com.r_buff[com.head] != 13) &&
	 (com.head != com.tail))
   {
     com.buffer[com.inp_count] = com.r_buff[com.head];
     com.head++;
     com.inp_count++;

     if (com.head == 128)
       com.head = 0;
   } /* end while */

  /* After all characters have been copied */
  if (*com.count_var == com.inp_count)
  {
    com.status = IDLE;
    *com.eflag_p = 1; /* set requestor's event flag */
    *com.count_var = com.inp_count; /* set requestor's count variable */
    com.inp_count = 0;
  }

  enable(); /* ring buffer copying done, so reenable interrupts */

  return 0;
}


/*************************************************************************************
 Procedure : com_write

 Purpose :

 The com_write function is used to initiate the transfer of a block of data to the serial port.

 Parameters :  buf_p is a pointer to the starting address of the buffer containing
	       the block of characters to be printed

	       count_p is the address of an integer count value indicating the number of characters to be printed

 Return Value :  0  -- No Error

 Errors :  COM_WR_NOT_OPEN  -- Port not open
	   COM_WR_INV_BUF  -- Invalid buffer address
	   COM_WR_INV_COUNT  -- Invalid count address or count value
	   COM_WR_BUSY  -- Device busy

 Functions used :  outportb, inportb

 Summary of Algorithm :

 Ensure that the input parameters are valid. Ensure that the port is currently open and idle. Install the buffer pointer and counters in the DCB, and set the current status to writing. Clear the caller's event flag. Get the first character from the requestor's buffer and store it in the output register. Enable write interrupts by setting bit 1 of the Interrupt Enable register.
*************************************************************************************/

int com_write(char *buf_p, int *count_p)
{
  if (com.ready_flag!=OPEN)
    return COM_WR_NOT_OPEN;

  if (buf_p==NULL)
    return COM_WR_INV_BUF;

  if ((count_p==NULL) || (*count_p < 0))
    return COM_WR_INV_COUNT;

  if (com.status!=IDLE)
    return COM_WR_BUSY;

  /*Install the buffer pointer and counters in the DCB*/
  com.out_trav = buf_p;
  com.out_count = count_p;

  com.status=WRITING;

  *com.eflag_p = 0; /* clear caller's event flag */
  com.ntransfer = 1;

  /* send first char from requestor's buffer to output register*/
  outportb(IN_OUT,*com.out_trav);
  com.out_trav++;

  outportb(INT_EN, inportb(INT_EN) | 0x02); /* enable write interrupts */

  return 0;
}


/*************************************************************************************
 Procedure : com_handler

 Purpose:

 The interrupt vector transfers initially to the first-level handler, which is responsible for determining the exact cause of the interrupt and performing some general processing. This handler in turn selects and calls the specific second-level handler appropriate for the specific interrupt.

 Parameters : None

 Returned Value : None

 Functions used : disable, enable, outportb, inportb, com_ouput, com_input

 Summary of Algorithm :

 The specific steps to be carried out by the first-level interrupt handler are as follows:

 If the port is not open, clear the interrupt and return. Read the Interrupt ID register to determine the exact cause of the interrupt. Bit 0 must be a 0 if the interrupt was actually caused by the serial port. In this case, bits 2 and 1 indicate the specific interrupt type as follows:

 Bit 2 Bit 1 Interrupt Type
 0     0     Modem Status Interrupt
 0     1     Output Interrupt
 1     0     Input Interrupt
 1     1     Line Status Interrupt

 Call the appropriate second-level handler. Clear the interrupt by sending EOI to the PIC command register.
*************************************************************************************/

void interrupt com_handler(void)
{
  /*If the port is not open, clear the interrupt and return*/
  if (com.ready_flag != OPEN) {
    outportb(PIC1, 0x20); /* End of Interrupt */
    return;
  }

  disable();

  /*Read register to determine exact cause of interrupt*/
  result = inportb(INT_ID_REG);

  if ((result & 0x01) != 0) /* interrupt not caused by serial port */
    return;

  result = result >> 1;

  if (result == 0) { /* indicates modem status interrupt */
    inportb(MODEM_STATUS);
  }

  if (result == 1) { /* indicates output interrupt */
    com_output();
  }

  if (result == 2) { /* indicates input interrupt */
    com_input();
  }

  if (result == 3) { /* indicates line status interrupt */
    inportb(LINE_STATUS);
  }

  outportb(PIC1, 0x20); /* End of Interrupt */
  enable();
}


/*************************************************************************************
 Procedure : com_input

 Purpose : Second level interrupt handler responsible for the input interrupts.

 Parameters : None

 Return Value : None

 Functions used : inportb

 Summmary of Algorithm :

 The second-level handler for the input interrupt should perform the following actions:

 Read a character from the input register. If the current status is not reading, store the character in the ring buffer. If the buffer is full, discard the character. In either case return to the first-level handler. Do not signal completion. Otherwise, the current status is reading. Store the character in the requestor's input buffer. If the count is not completed and the character is not CR, return. Do not signal completion. Otherwise, the transfer has completed. Set the status to idle. Set the event flag and return the requestor's count value.
*************************************************************************************/

void com_input(void)
{
  data_buff = inportb(IN_OUT); /* read a char from input register */

  if (com.status != READING) { /* then store the char in ring buffer */
    if (com.head != com.tail) { /* if ring buffer not full */
      com.r_buff[++com.tail]=data_buff;

      /*If the buffer is full, discard the character*/
      if (com.tail==128) com.tail=0; 
    }
  } else { /* current status is reading */

      if (data_buff != 13) { /* char not a carriage return (CR) */
	 *com.trav=data_buff; /* store in requestor's input buffer */
         com.trav++;
	 com.inp_count++;
      }

      /* if count is completed or char is CR, transfer is complete */
      if ((*com.count_var==com.inp_count) || (data_buff==13))
        {
           com.status=IDLE;
           *com.trav = '\0';  com.trav++;
           *com.trav = '\r';  com.trav++;
           com.trav = com.buffer;

	   *com.eflag_p=1; /* set requestor's event flag */
           *com.count_var=com.inp_count; /* set requestor's count */
	}
  }
}


/*************************************************************************************
 Procedure : com_output

 Purpose : Second level interrupt handler responsible for the output interrupts.

 Parameters : None

 Return Value : None

 Functions used : inportb, outportb

 Summary of Algorithm :

 The second-level handler for the output interrupt should perform the following actions:

 If the current status is not writing, ignore the interrupt and return. Otherwise, if the count has not been exhausted, get the next character from the requestor's output buffer and store it in the output register. Return without signaling completion. Otherwise, all characters have been transferred. Reset the status to idle. Set the event flag and return the count value. Disable write interrupts by clearing bit 1 in the interrupt enable register.
*************************************************************************************/

void com_output(void)
{
  if (com.status != WRITING)
     return; /* ignore interrupt */

  if (com.ntransfer < *com.out_count) { /* if character request not completed */
    outportb(IN_OUT,*com.out_trav); /* send next char */
    com.out_trav++;
    com.ntransfer++;
    return;
  } else { /* all chars have been transferred */
      com.status=IDLE;

      *com.eflag_p=1; /* set requestor's event flag */

      /* disable write interrupts by clearing bit 1 */
      outportb(INT_EN, inportb(INT_EN) & 0xFD);
  }
}
