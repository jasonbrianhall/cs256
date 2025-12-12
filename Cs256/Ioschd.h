/***********************************************************************

  IO Processing Controls


  File Name: ioschd.h

  Author:  Wes Queen
  Version: 6.0
  Date:    04/15/01

  Purpose: IO Control and Management

    This file defines IO control data stuctures, and related
    functions to manage IO.

 Functions:
    init_cb
    io_scheduler
    io_completion
    io_enqueue
    io_dequeue

*/

#ifndef _IO_H
#define _IO_H

/* IO request descriptor */
typedef struct IO_REQ {
  PCB    *process;     /* associated process control block */
  char   *buf_p;       /* location of transfer buffer */
  int    *count_p;     /* location of count variable of buffer */
  int    op_code;      /* operation to perform (ie. WRITING) */
 int    device_id;    /* IO device */
  struct IO_REQ *prev; /* previous in queue */
  struct IO_REQ *next; /* next in queue */
} IO_REQ;

typedef struct IO_QUEUE{
  IO_REQ *head;
  IO_REQ *tail;
} IO_QUEUE;

/* IO control block */
typedef struct IOCB {
  PCB      *car;     /* current active request */
  char     *buf_p;   /* location of transfer buffer */
  int      *count_p; /* location of count variable of buffer */
  int      op_code;  /* current operation of device */
  IO_QUEUE q;        /* device's IO waiting queue */
  int      eflag;    /* event flag to control this device */
} IOCB;

void init_cb(IOCB *cb);
int io_scheduler(PCB *process, int op_code, int device_id,
                 char *buf_p, int *count_p);
void io_completion(int device_id);
void io_enqueue(IO_QUEUE *q, IO_REQ *req);
IO_REQ *io_dequeue(IO_QUEUE *q);

#define IO_INV_OPCODE  (-501)
#define IO_INV_DEVICE  (-502)

#endif
