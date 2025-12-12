/***********************************************************************

IO Scheduler

File Name: ioschd.c

Author:  Wes Queen

Version: 6.0

Date:    04/15/01

Purpose: IO Control and Management


This file defines IO control data stuctures, and related
functions to manage IO.


Functions: init_cb,io_scheduler,io_completion,io_enqueue,io_dequeue

*/

#include "mpx_supt.h"
#include "pcb_cont.h"
#include "devdrive.h"
#include "ioschd.h"
#include "trmdrive.h"

IOCB print_control_block;
IOCB com_control_block;
IOCB term_control_block;

extern Header Blocked_queue,Ready_queue,Ready_susp_queue,Blocked_susp_queue;


/*  Procedure: init_cb
*
*  Purpose: initialize an IO control block
*
*  Parameters: cb  --IOCB to be initialized
*
*  Return value: none
*
*  Calls: none
*
*  Globals: none
*
*  Errors: none
*
*/
void init_cb(IOCB *cb)
{
	cb->car    = NULL;
	cb->q.head = NULL;
	cb->q.tail = NULL;
	cb->eflag  = 0;
}


/*  Procedure: io_scheduler
*
*  Purpose: process input and output requests
*
*  Parameters: process   --PCB pointer to requesting process
*              op_code   --operation to be performed (eg. READ)
*              device_id --id of device for operation (eg. COM_PORT)
*              buf_p     --pointer to requestor's buffer
*              count_p   --pointer to requestor's length of buffer
*
*  Return value: 0             if successful
*                IO_INV_OPCODE if op_code is invalid (device dependent)
*                IO_INV_DEVICE if device_id is unknown
*
*  Calls: prt_write
*         com_write
*         com_read
*         sys_alloc_mem
*         io_enqueue
*         block
*
*  Globals: com_control_block
*           print_control_block
*
*  Errors: none
*
*/
int io_scheduler(PCB *process, int op_code, int device_id,
char *buf_p, int *count_p)
{
	IO_REQ *new_req;
	PCB *qnode;

	/* must be a valid operation */
	if (op_code != READ && op_code != WRITE && op_code != CLEAR)
	return IO_INV_OPCODE;

	/* must be a valid device */
	if (device_id != COM_PORT && device_id != PRINTER && device_id != TERMINAL)
	return IO_INV_DEVICE;

	/* can only write to the printer */
	if (device_id == PRINTER && op_code != WRITE)
	return IO_INV_OPCODE;

	/* can only read and write to com port */
	if (device_id == COM_PORT && op_code != WRITE && op_code != READ)
	return IO_INV_OPCODE;

	/* can only read and write to terminal */
	if (device_id == TERMINAL && op_code != WRITE && op_code != READ && op_code != CLEAR)
	return IO_INV_OPCODE;

	switch(device_id)
	{
		case PRINTER:

		/* check status of device */
		if (print_control_block.car == NULL) {
			/* no current process, so start request immediately */
			print_control_block.car = process;
			print_control_block.buf_p = buf_p;
			print_control_block.count_p = count_p;

			switch (op_code)
			{
				case WRITE:
				prt_write (print_control_block.buf_p, print_control_block.count_p);
				break;
				default:
				break;
			}

		} else { /* device is busy. put request in waiting queue */
			/* allocate memory and initialize incoming IO request */
			new_req = (IO_REQ *) sys_alloc_mem(sizeof(IO_REQ));
			new_req->process = process;
			new_req->buf_p = buf_p;
			new_req->count_p = count_p;
			new_req->op_code = op_code;
			new_req->device_id = device_id;
			io_enqueue(&print_control_block.q, new_req);
		}
		break;

		case COM_PORT:

		/* check status of device */
		if (com_control_block.car == NULL) {
			/* no current process, so start request immediately */
			com_control_block.car = process;
			com_control_block.buf_p = buf_p;
			com_control_block.count_p = count_p;

			switch (op_code)
			{
				case WRITE:
				com_write (com_control_block.buf_p, com_control_block.count_p);
				com_control_block.op_code = WRITE;
				break;
				case READ:

				com_read(com_control_block.buf_p, com_control_block.count_p);
				com_control_block.op_code = READ;
				break;
				default:
				break;
			}

		} else { /* device is busy. put request in waiting queue */
			/* allocate memory and initialize incoming IO request */
			new_req = (IO_REQ *) sys_alloc_mem(sizeof(IO_REQ));
			new_req->process = process;
			new_req->buf_p = buf_p;
			new_req->count_p = count_p;
			new_req->op_code = op_code;
			new_req->device_id = device_id;
			io_enqueue(&com_control_block.q, new_req);
		}
		break;

		case TERMINAL:


		/* check status of device */
		if (term_control_block.car == NULL) {
			/* no current process, so start request immediately */
			term_control_block.car = process;
			term_control_block.buf_p = buf_p;
			term_control_block.count_p = count_p;

			switch (op_code)
			{
				case WRITE:
				trm_write(term_control_block.buf_p, term_control_block.count_p);

				break;
				case READ:
				trm_read(term_control_block.buf_p, term_control_block.count_p);
				break;

				case CLEAR:
				trm_clear();
				break;
				default:
				break;
			}

		} else { /* device is busy. put request in waiting queue */

			/* allocate memory and initialize incoming IO request */
			new_req = (IO_REQ *) sys_alloc_mem(sizeof(IO_REQ));
			new_req->process = process;
			new_req->buf_p = buf_p;
			new_req->count_p = count_p;
			new_req->op_code = op_code;
			new_req->device_id = device_id;

			io_enqueue(&term_control_block.q, new_req);
		}
		break;


		default: /* should never reach here */

		break;
	}

	qnode=(PCB*)Find_PCB(&Blocked_queue,process->Process_Name);
	if(qnode==NULL)
	{
		process->Process_State[0]=BLOCKED;
		Insert_PCB(process,&Blocked_queue,FIFOQ);
	}


	trm_getc();
	return 0;
}



/*  Procedure: io_completion
*
*  Purpose: upon IO completion, switch process to ready and get next one
*
*  Parameters: device_id --id of device for operation (eg. COM_PORT)
*
*  Return value: none
*
*  Calls: enqueue
*         io_dequeue
*         io_scheduler
*         sys_free_mem
*         unblock
*
*  Globals: com_control_block
*           print_control_block
*           term_control_block
*
*  Errors: none
*
*/
void io_completion(int device_id)

{

	IO_REQ *req;
	PCB *qnode;
	PCB *Temp_PCB;
	extern Header Blocked_queue;
	extern Header Blocked_susp_queue;
	qnode = NULL;
	switch (device_id)
	{
		case PRINTER:
		Unblock_PCB(print_control_block.car->Process_Name);

		/* clear active request signalling a idle device */
		print_control_block.car = NULL;
		print_control_block.buf_p = NULL;
		print_control_block.count_p = NULL;
		print_control_block.eflag = 0; /* reset event flag */

		/* search for next process in waiting list */
		req = io_dequeue(&print_control_block.q);


		if (req != NULL) {
			qnode = (PCB*) Find_PCB(&Blocked_queue,req->process->Process_Name);
			if (qnode != NULL){
			Remove_PCB(&Blocked_queue,qnode->Process_Name);
			sys_free_mem(qnode);
			}
			if (qnode == NULL)
			{
			qnode = (PCB*) Find_PCB(&Blocked_susp_queue,req->process->Process_Name);
			Remove_PCB(&Blocked_susp_queue,qnode->Process_Name);
			sys_free_mem(qnode);
			}
			io_scheduler(req->process, req->op_code, req->device_id,
			req->buf_p, req->count_p);
			sys_free_mem(req);
		}
		break;

		case COM_PORT:
		Unblock_PCB(com_control_block.car->Process_Name);

		/* clear active request signalling a idle device */
		com_control_block.car = NULL;
		com_control_block.buf_p = NULL;
		com_control_block.count_p = NULL;
		com_control_block.eflag = 0; /* reset event flag */

		/* search for next process in waiting list */
		req = io_dequeue(&com_control_block.q);

		if (req != NULL) {
			qnode = (PCB*) Find_PCB(&Blocked_queue,req->process->Process_Name);
			if (qnode != NULL){
			Remove_PCB(&Blocked_queue,qnode->Process_Name);
			sys_free_mem(qnode);
			}

			if (qnode == NULL)
			{
			qnode = (PCB*) Find_PCB(&Blocked_susp_queue,req->process->Process_Name);
			Remove_PCB(&Blocked_susp_queue,qnode->Process_Name);
			sys_free_mem(qnode);
			}
			io_scheduler(req->process, req->op_code, req->device_id,
			req->buf_p, req->count_p);
			sys_free_mem(req);
		}
		break;

		case TERMINAL:

		Unblock_PCB(term_control_block.car->Process_Name);

		/* clear active request signalling a idle device */
		term_control_block.car = NULL;
		term_control_block.buf_p = NULL;
		term_control_block.count_p = NULL;
		term_control_block.eflag = 0; /* reset event flag */

		/* search for next process in waiting list */
		req = io_dequeue(&term_control_block.q);
		if (req != NULL) {
			qnode = (PCB*)Find_PCB(&Blocked_queue,req->process->Process_Name);
			if (qnode != NULL){
			Remove_PCB(&Blocked_queue,qnode->Process_Name);
			sys_free_mem(qnode);
			}

			if (qnode == NULL)
			{
			qnode =(PCB*) Find_PCB(&Blocked_susp_queue,req->process->Process_Name);
			Remove_PCB(&Blocked_susp_queue,qnode->Process_Name);
			sys_free_mem(qnode);
			}
			io_scheduler(req->process, req->op_code, req->device_id,
			req->buf_p, req->count_p);
			sys_free_mem(req);
		}
		break;

		default:

		break;
	}
}


/*  Procedure: io_enqueue
*
*  Purpose: add a request to the IO queue
*
*  Parameters: q   --this is IO queue to insert into
*              req --this is the request to put in q
*
*  Return value: none
*
*  Calls: none
*
*  Globals: none
*
*  Errors: none
*/
void io_enqueue(IO_QUEUE *q, IO_REQ *req)
{
	req->prev = NULL;
	req->next = NULL;

	/* insert into the tail of the queue */
	if (q->head == NULL) { /* if queue is empty, make req head of queue */
		q->head = req;
		q->tail = req;
	} else {  /* insert into tail */
		q->tail->next = req;
		req->prev = q->tail;
		q->tail = req;
	}
}


/*  Procedure: io_dequeue
*
*  Purpose: Remove the first item from a given queue.
*
*  Parameters: q --this is queue to get the first item
*
*  Return value: NULL if queue is empty
*                address of first IO_REQ if not empty
*
*  Calls: none
*
*  Globals: none
*
*  Errors: none
*
*/
IO_REQ *io_dequeue(IO_QUEUE *q)
{
	IO_REQ *ret = NULL; //pointer to be returned
	Process_List *qnode;

	ret = q->head;

	if (ret != NULL) {
		q->head = ret->next;
		if (q->head != NULL)
		q->head->prev = NULL;
	}
	return ret;
}

