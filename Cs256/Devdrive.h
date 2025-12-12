/***********************************************************************
  File Name:  devdrive.h

  Author:  Wes Queen
  Version: 6.0
  Date:    04/13/01
  Purpose: Header file for Drivers

    This file contains all the data structures, defines used by various
    functions, and all function prototypes for the devices supported
    by the MPX OS.

  Functions:  prt_open,prt_close,prt_write,com_open,com_close,com_read
              com_write


************************************************************************/

#ifndef _DRIVERS_H
#define _DRIVERS_H

/* Device Control Block */
typedef struct DCB {
  int ready_flag;  /* device is UNUSED, OPEN, or CLOSED */
  int status;      /* device is IDLE, READING, or WRITING */
  int *eflag_p;    /* event flag pointer */
  char *buffer;    /* requestor's buffer */
  int current_loc; /* current location in requestor's buffer */
  int buf_size;    /* total buffer size */
  int ntransfer;   /* number of characters already transferred */
  int *count_var;  /* address of requestor's count variable */
  char r_buff[128];/* ring buffer for typeahead */
  int head;        /* head of the ring buffer */
  int tail;        /* tail of the ring buffer */
  int inp_count;   /* number of characters transferred out of ring buffer */
  int *out_count;
  char *trav;
  char *out_trav;
} DCB;


/* DCB availability (ready_flag) */
#define UNUSED  0
#define OPEN    1
#define CLOSED  2

/* DCB status */
#define IDLE     0
#define READING  1
#define WRITING  2


/* Printer (Parallel Port) related defines */

#define PORTADDRESS  0x378  /* Enter the Port Address here */
#define IRQ 7               /* IRQ here */

#define DATA     (PORTADDRESS+0)
#define STATUS   (PORTADDRESS+1)
#define CONTROL  (PORTADDRESS+2)

#define PIC1   0x20
#define PIC2   0xA0

/* prt_open: open the printer */
/*   RETURNS: error code, or zero if ok */
int prt_open (int *eflag_p);


/* prt_close: close the printer */
/*   RETURNS: error code, or zero if ok */
int prt_close (void);

/* prt_write: begin block output */
/*   RETURNS: error code, or zero if ok */
int prt_write (char *buf_p, int *count_p);


/* Printer Error Codes */

/* prt_open error codes */
#define PRT_INV_EFLAG  (-101)  /* invalid (null) event flag pointer */
#define PRT_ALR_OPEN   (-102)  /* printer already open */

/* prt_close error codes */
#define PRT_NOT_OPEN   (-201)  /* printer not open */

/* prt_write error codes */
#define PRT_WR_NOT_OPEN (-401) /* printer not open */

#define PRT_INV_BUF     (-402) /* invalid buffer address */
#define PRT_INV_COUNT   (-403) /* invalid count address or count value */
#define PRT_BUSY        (-404) /* printer busy */


/* Serial (COM1) related defines */

#define INT_ID  0x0C

/*Define ACC Registers*/
#define BASE          0x3f8
#define IN_OUT        BASE
#define LOW           BASE
#define HIGH          BASE+1
#define INT_EN        BASE+1
#define INT_ID_REG    BASE+2
#define LINE_CONTROL  BASE+3
#define MODEM_CONTROL BASE+4
#define LINE_STATUS   BASE+5
#define MODEM_STATUS  BASE+6

/* com_open: open the COM port */
/*   RETURNS: error code, or zero if ok */
int com_open(int *eflag_p,int baud_rate);

/* com_close: close the COM port */
/*   RETURNS: error code, or zero if ok */
int com_close(void);

/* com_write: begin block output */
/*   RETURNS: error code, or zero if ok */
int com_write(char *buf_p,int *count_p);

/* com_read: begin block input */
/*   RETURNS: error code, or zero if ok */
int com_read(char *buf_p,int *count_p);


/* Serial Port Error Codes */

/* com_open error codes */
#define COM_INV_EFLAG  (-101)  /* invalid (null) event flag pointer */
#define COM_INV_BAUD   (-102)  /* invalid baud rate divisor */
#define COM_ALR_OPEN   (-103)  /* port already open */

/* com_close error codes */
#define COM_NOT_OPEN   (-201)  /* serial port not open */

/* com_read error codes */
#define COM_RD_NOT_OPEN  (-301) /* serial port not open */
#define COM_RD_INV_BUF   (-302) /* invalid buffer address */
#define COM_RD_INV_COUNT (-303) /* invalid count address or count value */
#define COM_RD_BUSY      (-304) /* device busy */

/* com_write error codes */
#define COM_WR_NOT_OPEN  (-401) /* serial port not open */

#define COM_WR_INV_BUF   (-402) /* invalid buffer address */
#define COM_WR_INV_COUNT (-403) /* invalid count address or count value */
#define COM_WR_BUSY      (-404) /* device busy */

#endif

