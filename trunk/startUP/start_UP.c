/*
 * start_UP.c
 *
 *  Created on: Apr 21, 2013
 *      Author: George Okeke
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <mqueue.h>
#include <string.h>
#include "Hoverboard_Defines.h"

mqd_t qd;
int shm_fd;
sem_t *semdes = SEM_FAILED;
struct mq_attr queueAttr;
DataStructure *sensorsData;


int main(int argc, char *argv[])
{
	// set attributes for the queue
	queueAttr.mq_maxmsg = 16;
	queueAttr.mq_msgsize = COMMANDSIZE;

	// create a message queue for receiving command
	if ((qd = mq_open(QUEUENAME, O_CREAT | O_RDWR | O_EXCL , 0777, &queueAttr)) == (mqd_t)-1)
	{
		if ((qd = mq_open(QUEUENAME, O_RDWR)) == -1)
		{
			perror("Error: reopening the queue");
			//return 0;
		}
	}

	// creeer de data shared memory
	if ((shm_fd = shm_open(SHAREDATA, O_CREAT | O_RDWR | O_EXCL, 0777)) == -1) {
		if ((shm_fd = shm_open(SHAREDATA, O_RDWR, 0666)) == -1) {
			perror("Error: cannot reopen shm");
			return 0;
		}
	}
	// set the shared memory size
	if (ftruncate(shm_fd, DATASIZE) != 0) {
		perror("Error: cannot set memory size");
		return 0;
	}
	// map shared memory in address space
	if ((sensorsData = mmap(0, DATASIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0)) == MAP_FAILED) {
		perror("Error: cannot map memory");
		return 0;
	}

	// creer de semaphore voor de shared memory
	semdes = sem_open(SEM_SHAREDATA, O_CREAT | O_EXCL, 0777, 1);
	if (semdes == SEM_FAILED) {
		if ( (semdes = sem_open(SEM_SHAREDATA, 0)) == SEM_FAILED) {
			perror("Error: reopening the sem");
			return 0;
		}
	}

	// After creating the Shared memory, Message queue and semaphore
	// The while loop will keep the program running.
	// Comment the while loop if the program should exit after creating
	// the resources.
//	while (1) {
//		sleep(1);
//	}

	return (0);
}
