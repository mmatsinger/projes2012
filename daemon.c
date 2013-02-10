/**
 * Created on: Jan 4, 2013
 * Author: George Okeke
 */

//#include <libusb.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <mqueue.h>
#include <signal.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include "Hoverboard_Defines.h"

//-------Threads functions-----------------
void* sendCommandTestHandler(void* arg);
void* motorControlHandler(void* arg);
//-----------------------------------------

//------General functions------------------
int gpio_export(unsigned int gpio);
int gpio_unexport(unsigned int gpio);
int gpio_set_dir(unsigned int gpio, unsigned int out_flag);
int gpio_set_value(unsigned int gpio, unsigned int value);
int gpio_get_value(unsigned int gpio, unsigned int *value);
int gpio_set_edge(unsigned int gpio, char *edge);
int gpio_fd_open(unsigned int gpio);
int gpio_fd_close(int fd);
//------------------------------------------

int main(int argc, char *argv[])
{
	unsigned char toggleLiftMotor = 1; // variable om de lift motor (aan = 1) en (uit = 0) te zetten
	unsigned char switch_ON = 1; // bepaalt of dit main programma beindigd wordt (bij een waarde 0 wordt het hele programma beindigd).
	ControlState givingState;	// bevat de huidige aangeven besturen commando waardes
	mqd_t qd;
	unsigned int priority = 1;
	struct mq_attr queueAttr;
	char buf[sizeof(CommandStructure)];
	CommandStructure *receivedCommand;
	DataStructure *sensorsData;
	int shm_fd;
	sem_t *semdes = SEM_FAILED;
	pthread_t motorControlThread;	// thread voor het besturen van de motoren
	pthread_t commandTestThread;	// alleen voor test doeleind.
	unsigned int liftMotorGPIO;

	// set attributes for the queue
	queueAttr.mq_maxmsg = 16;
	queueAttr.mq_msgsize = COMMANDSIZE;
	
	// for test purposes
	printf("queue name: %s\n", QUEUENAME);

	// initialiseer de lift motor
	liftMotorGPIO = 138;	// De GPIO om de lift motor aan te sturen.
	gpio_export(liftMotorGPIO);
	gpio_set_dir(liftMotorGPIO, DIROUT);
	gpio_set_value(liftMotorGPIO, 1);
	
	// create a message queue for receiving command
	if ((qd = mq_open(QUEUENAME, O_CREAT | O_RDWR | O_EXCL , 0666, &queueAttr)) == (mqd_t)-1) {
		if ((qd = mq_open(QUEUENAME, O_RDWR)) == -1) {
			perror("Error: reopening the queue");
			//return 0;
		}
	}

	// creeer de data shared memory
	if ((shm_fd = shm_open(SHAREDATA, O_CREAT | O_RDWR | O_EXCL, 0666)) == -1) {
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
	semdes = sem_open(SEM_SHAREDATA, O_CREAT | O_EXCL, 0666, 1);
	if (semdes == SEM_FAILED) {
		if ( (semdes = sem_open(SEM_SHAREDATA, 0)) == SEM_FAILED) {
			perror("Error: reopening the sem");
			return 0;
		}
	}

	// De initialisatie van het rotate en speed motor wordt binnen de
	//		besturen programma thread gedaan.
	// Start de besturen programma thread
	pthread_create(&motorControlThread, NULL, &motorControlHandler, (void *)&givingState);

	// creer een sendCommand thread. Dit thread is alleen voor het test
	//		doeleind, tenminste tot dat Roy een werkende webpage af heeft.
	pthread_create(&commandTestThread, NULL, &sendCommandTestHandler, NULL);

	// Start de sensors programma thread
	// Joppe: zet a.u.b jouw sensors programma thread(s) hieronder

	// Bijna aan het einde van dit main programma wordt een while loop gestart
	// 		die receive functie van het message queue uitvoert. De lift motor
	//		wordt afgehandeled binnen de message queue gedeelte
	while(switch_ON)
	{
		printf("waiting for an input\n");
		if ((mq_receive(qd, buf, sizeof(CommandStructure), &priority)) == (mqd_t)-1) {
			perror("Error: cannot receive message");
		}

		receivedCommand = (CommandStructure*)buf;

		if (receivedCommand->command == LiftHoverboard) { // commando is "Lift Hoverboard"
			if (toggleLiftMotor == OFF_STATE) {
				toggleLiftMotor = ON_STATE;
				gpio_set_value(liftMotorGPIO, 1);	// zet de lift motor aan
			}
			else {
				toggleLiftMotor = OFF_STATE;
				gpio_set_value(liftMotorGPIO, 0);	// zet de lift motor uit
			}
		}
		else if (receivedCommand->command == Hold_Speed_Direction) { // commando is "Hold speed direction"
			if (givingState.hold_speed_direction == OFF_STATE) {
				givingState.hold_speed_direction = ON_STATE;
			}
			else {
				givingState.hold_speed_direction = OFF_STATE;
			}
		}
		else if (receivedCommand->command == ActivateAutomaticControl) { // commando is "Activate automatic control"
			if (givingState.automaticControl == OFF_STATE) {
				givingState.automaticControl = ON_STATE;
			}
			else {
				givingState.automaticControl = OFF_STATE;
			}
		}
		else if (receivedCommand->command == move) { // commando is "move"
			givingState.moveSpeed = receivedCommand->commandValue;
		}
		else if (receivedCommand->command == rotate) { // commando is "rotate"
			givingState.rotateAngle = receivedCommand->commandValue;
		}
		else if (receivedCommand->command == termninate_prog) { // commando is "termninate_prog"
			switch_ON = 0;
		}
		else {
			printf("No selection match, please give the right selection\n");
		}

		//sleep(4);
	}

	// zorgt dat de lift motor uitgaat als het programma beeindigd wordt.
	gpio_set_value(liftMotorGPIO, 0);	// zet de lift motor uit
	gpio_unexport(liftMotorGPIO);
	
	// Voordat de main programma eindigd wordt, zorg dat
	// 		alle gecreerde threads zijn allemaal gestopt.
	//		De message queue, shared memory en semaphore zijn allemaal geclosed en unlinked.
	// stop alle gecreerde threads
	pthread_cancel(motorControlThread);
	pthread_cancel(commandTestThread);
	pthread_join(motorControlThread, NULL);
	pthread_join(commandTestThread, NULL);

	// close message queue
	printf("clossing the command message queue\n");
	if (mq_close(qd) == -1) {
		perror("Error: closing queue");
	}
	// unlink message queue
	printf("unlinking the command message queue\n");
	if (mq_unlink(QUEUENAME) == -1) {
		perror("Error: unlinking message queue");
	}

	printf("closing the data shared memory\n");
	close(shm_fd);
	// unmap shared memory from address space
	printf("unmapping the data shared memory\n");
	munmap(sensorsData, DATASIZE);
	// remove shared memory segment
	printf("unlinking the data shared memory\n");
	shm_unlink(SHAREDATA);

	// close semaphore
	printf("Closing the semaphore\n");
	if (sem_close(semdes) != 0) {
		perror("Error: sem_close failed");
	}
	// unlink semaphore
	printf("unlinking the semaphore\n");
	if (sem_unlink(SEM_SHAREDATA) != 0) {
		perror("Error: sem_unlink failed");
	}

	return (0);
}


void* sendCommandTestHandler(void* arg)
{
	unsigned int priority = 1;
	CommandStructure sendData;
	mqd_t qd;
	char input[2];
	int tempCommand;

	// open a message queue named "contol_queue"
	if ((qd = mq_open(QUEUENAME, O_WRONLY)) == -1) {
		perror("Error: opening the queue from sendCommandTestHandler");
		return NULL;
	}

	while (1)
	{
		// lees input waardes
		printf("Command list:\n");
		printf("Lift Hoverboard: 			1\n");
		printf("Hold speed/direction: 		2\n");
		printf("Activate automatic control: 3\n");
		printf("move:						4\n");
		printf("rotate:						5\n\n");
		printf("Enter a command: \n");
		scanf("%s", input);
		tempCommand = atoi(input);

		sendData.command = (unsigned char)tempCommand;
		if (tempCommand == 0x04) { // commando is "move"
			printf("Enter speed value: \n");
			scanf("%s", input);
			tempCommand = atoi(input);
		}
		else if (tempCommand == 0x05) { // commando is "rotate"
			printf("Enter rotate value: \n");
			scanf("%s", input);
			tempCommand = atoi(input);
		}
		sendData.commandValue = (unsigned char)tempCommand;

		// stuurt de waarde naar de message queue
		if (mq_send(qd, (char *)&sendData, COMMANDSIZE, priority) == -1) {
			perror("Error: cannot send message");
		}
	}

	// close message queue
	if (mq_close(qd) == -1) {
		perror("Error: closing queue");
	}

	return (NULL);
}


void* motorControlHandler(void* arg)
{
	ControlState *givingState = (ControlState *)arg;	// bevat de huidige aangeven besturen commando waardes

	// initialiseer de rotate motor in een default positie
	// initialiseer de speed motor in een default aandrijf kracht

	return 0;
}

/**
 * gpio export
 */
int gpio_export(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];
	
	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0) {
		perror("Error: gpio/export");
		return fd;
	}
	
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
	
	return 0;
}

/**
 * gpio unexport
 */
int gpio_unexport(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];
	
	fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0) {
		perror("Error: gpio/unexport");
		return fd;
	}
	
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
	
	return 0;
}

/**
 * gpio_set_dir
 */
int gpio_set_dir(unsigned int gpio, unsigned int out_flag)
{
	int fd, len;
	char buf[MAX_BUF];
	
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/direction", gpio);
	
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("Error: gpio/direction");
		return fd;
	}
	
	if (out_flag == DIROUT) {
		write(fd, "out", 4);
	}
	else {
		write(fd, "in", 3);
	}
	close(fd);
	
	return 0;
}

/**
 * gpio_set_value
 */
int gpio_set_value(unsigned int gpio, unsigned int value)
{
	int fd, len;
	char buf[MAX_BUF];
	
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
	
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("Error: gpio/set-value");
		return fd;
	}
	
	if (value) {
		write(fd, "1", 2);
	}
	else {
		write(fd, "0", 2);
	}
	close(fd);
	
	return 0;
}

/**
 * gpio_get_value
 */
int gpio_get_value(unsigned int gpio, unsigned int *value)
{
	int fd, len;
	char buf[MAX_BUF];
	char ch;
	
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
	
	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		perror("Error: gpio/get-value");
		return fd;
	}
	
	read(fd, &ch, 1);
	
	if (ch != '0') {
		*value = 1;
	}
	else {
		*value = 0;
	}
	close(fd);
	
	return 0;
}

/**
 * gpio_set_edge
 */
int gpio_set_edge(unsigned int gpio, char *edge)
{
	int fd, len;
	char buf[MAX_BUF];
	
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);
	
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("Error: gpio/set-edge");
		return fd;
	}
	
	write(fd, edge, strlen(edge) + 1);
	close(fd);
	
	return 0;
}

/**
 * gpio_fd_open
 */
int gpio_fd_open(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];
	
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
	
	fd = open(buf, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		perror("Error: gpio/fd_open");
	}
	
	return 0;
}

/**
 * gpio_fd_close
 */
int gpio_fd_close(int fd)
{
	return close(fd);
}
