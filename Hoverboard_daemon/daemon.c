/**
 * Created on: Jan 4, 2013
 *
 * Author: George Okeke
 */

//#include <libusb.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/fcntl.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <mqueue.h>
#include <signal.h>
#include <assert.h>
#include <string.h>
#include "Hoverboard_Defines.h"
#include "GPIO_Functions.h"
#include "serial.h"
#include "I2CFunctions.h"

//-------Threads functions-----------------
void* sendCommandTestHandler(void* arg);
void* motorControlHandler(void* arg);
void* distanceReadHandler(void* arg);
//-----------------------------------------

//-------Normal functions-------------------
unsigned char ser_send_verify(int fd, unsigned char sendBuf[]);
unsigned char calculate_Speed_Value(unsigned char speedPercentage);
int calculate_Rotate_Value(unsigned char rotatePercentage);
//--------------------------------

RC5_commando RC5_cmd;
unsigned char toggleLiftMotor = 1; // variable om de lift motor (aan = 1) en (uit = 0) te zetten
unsigned char switch_ON = 1; // bepaalt of dit main programma beindigd wordt (bij een waarde 0 wordt het hele programma beindigd).
ControlState givingState;	// bevat de huidige aangeven besturen commando waardes

unsigned int priority = 1;

char buf[sizeof(CommandStructure)];
CommandStructure *receivedCommand;
unsigned int liftMotorGPIO;

int main(int argc, char *argv[])
{
	DataStructure *sensorsData;
	mqd_t qd;
	int shm_fd;
	sem_t *semdes = SEM_FAILED;
	pthread_t motorControlThread;	// thread voor het besturen van de motoren
	pthread_t commandTestThread;	// alleen voor test doeleind.
	pthread_t distanceReadThread;   // thread voor uitlezen ultrasoon afstands sensor (srf02)

	// initialiseer de lift motor
	liftMotorGPIO = 139;	// De GPIO om de lift motor aan te sturen.
	gpio_export(liftMotorGPIO);
	gpio_set_dir(liftMotorGPIO, DIROUT);
	gpio_set_value(liftMotorGPIO, 1);
	
	// open a message queue for receiving command
	if ((qd = mq_open(QUEUENAME, O_RDWR)) == -1)
	{
		perror("Error: reopening the queue");
		//return 0;
	}

	// open de data shared memory
	if ((shm_fd = shm_open(SHAREDATA, O_RDWR, 0666)) == -1) {
		perror("Error: cannot reopen shm");
		return 0;
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

	// open de semaphore voor de shared memory
	if ( (semdes = sem_open(SEM_SHAREDATA, 0)) == SEM_FAILED) {
		perror("Error: reopening the sem");
		return 0;
	}

	// De initialisatie van het rotate en speed motor wordt binnen de
	//		besturen programma thread gedaan.
	// Start de besturen programma thread
	pthread_create(&motorControlThread, NULL, &motorControlHandler, (void *)&givingState);

	// creer een sendCommand thread. Dit thread is alleen voor het test
	//		doeleind, tenminste tot dat Roy een werkende webpage af heeft.
	pthread_create(&commandTestThread, NULL, &sendCommandTestHandler, NULL);

	// Start de sensors programma thread
	// Thread initialiseert I2C sensor en geeft afstand output elke 3 seconde
	//--------------------
	// Joppe: start a.u.b jouw sensors programma thread(s) hieronder
	pthread_create(&distanceReadThread, NULL, &distanceReadHandler, NULL);

	// Bijna aan het einde van dit main programma wordt een while loop gestart
	// 		die receive functie van het message queue uitvoert. De lift motor
	//		wordt afgehandeled binnen de message queue gedeelte
	while(switch_ON)
	{
		if ((mq_receive(qd, buf, sizeof(CommandStructure), &priority)) == (mqd_t)-1)
		{
			perror("Error: cannot receive message");
		}

		receivedCommand = (CommandStructure*)buf;

		if (receivedCommand->command == LiftHoverboard)
		{ // commando is "Lift Hoverboard"
			if (toggleLiftMotor == OFF_STATE)
			{
				toggleLiftMotor = ON_STATE;
				gpio_set_value(liftMotorGPIO, ON_STATE);	// zet de lift motor aan
			}
			else
			{
				toggleLiftMotor = OFF_STATE;
				gpio_set_value(liftMotorGPIO, OFF_STATE);	// zet de lift motor uit
			}
		}
		else if (receivedCommand->command == Hold_Speed_Direction)
		{ // commando is "Hold speed direction"
			if (givingState.hold_speed_direction == OFF_STATE)
			{
				givingState.hold_speed_direction = ON_STATE;
			}
			else
			{
				givingState.hold_speed_direction = OFF_STATE;
			}
		}
		else if (receivedCommand->command == ActivateAutomaticControl)
		{ // commando is "Activate automatic control"
			if (givingState.automaticControl == OFF_STATE)
			{
				givingState.automaticControl = ON_STATE;
			}
			else
			{
				givingState.automaticControl = OFF_STATE;
			}
		}
		else if (receivedCommand->command == move)
		{ // commando is "move"
			givingState.moveSpeed = receivedCommand->commandValue;
		}
		else if (receivedCommand->command == rotate)
		{ // commando is "rotate"
			givingState.rotateAngle = receivedCommand->commandValue;
		}
		else if (receivedCommand->command == termninate_prog)
		{ // commando is "termninate_prog"
			switch_ON = 0;
		}
		else
		{
			printf("No selection match, please give the right selection\n");
		}
	}

	// zorgt dat de lift motor uitgaat als het programma beeindigd wordt.
	gpio_set_value(liftMotorGPIO, OFF_STATE);	// zet de lift motor uit
	gpio_set_dir(liftMotorGPIO, DIRIN);	//stell de gpio als input (dit is voor de beveiligende reden)
	gpio_unexport(liftMotorGPIO);
	
	// Voordat de main programma eindigd wordt, zorg dat
	// 		alle gecreerde threads zijn allemaal gestopt.
	//		De message queue, shared memory en semaphore zijn allemaal geclosed en unlinked.
	// stop alle gecreerde threads
	pthread_cancel(motorControlThread);
	pthread_cancel(commandTestThread);
	pthread_cancel(distanceReadThread);
	pthread_join(motorControlThread, NULL);
	pthread_join(commandTestThread, NULL);
	pthread_join(distanceReadThread, NULL);

	// close message queue
	if (mq_close(qd) == -1) {
		perror("Error: closing queue");
	}
	// unlink message queue
	if (mq_unlink(QUEUENAME) == -1) {
		perror("Error: unlinking message queue");
	}

	close(shm_fd);
	// unmap shared memory from address space
	munmap(sensorsData, DATASIZE);
	// remove shared memory segment
	shm_unlink(SHAREDATA);

	// close semaphore
	if (sem_close(semdes) != 0)
	{
		perror("Error: sem_close failed");
	}
	// unlink semaphore
	if (sem_unlink(SEM_SHAREDATA) != 0)
	{
		perror("Error: sem_unlink failed");
	}

	return (0);
}

/**
 * Alleen bedoeled voor het testen tot dat Roy klaar is met de web page
 */
void* sendCommandTestHandler(void* arg)
{
	unsigned int priority = 1;
	CommandStructure sendData;
	mqd_t qd;
	char input[5];
	int tempCommand;

	// open a message queue named "contol_queue"
	if ((qd = mq_open(QUEUENAME, O_WRONLY)) == -1)
	{
		perror("Error: opening the queue from sendCommandTestHandler");
		return NULL;
	}

	while (1)
	{
		// lees input waardes
		printf("Command list:\n");
		printf("Lift Hoverboard: 	1\n");
		printf("Hold speed/direction: 	2\n");
		printf("Activate automatic control: 	3\n");
		printf("move:		4\n");
		printf("rotate:		5\n");
		printf("termninate_prog:	6\n\n");
		printf("Enter a command: \n");
		scanf("%s", input);
		tempCommand = atoi(input);
		printf("Enter rotate value: \n");

		sendData.command = (unsigned char)tempCommand;
		if (tempCommand == 0x04)
		{ // commando is "move"
			printf("Enter speed value: \n");
			scanf("%s", input);
			tempCommand = atoi(input);
		}
		else if (tempCommand == 0x05)
		{ // commando is "rotate"
			printf("Enter rotate value: \n");
			scanf("%s", input);
			tempCommand = atoi(input);
		}
		sendData.commandValue = (unsigned char)tempCommand;

		// stuurt de waarde naar de message queue
		if (mq_send(qd, (char *)&sendData, COMMANDSIZE, priority) == -1)
		{
			perror("Error: cannot send message");
		}
	}

	// close message queue
	if (mq_close(qd) == -1)
	{
		perror("Error: closing queue");
	}

	return (NULL);
}

// Thread besturen functie voor de servo's en de DC Motor
void* motorControlHandler(void* arg)
{
	ControlState *givingState = (ControlState *)arg;	// bevat de huidige aangeven besturen commando waardes
	char * tty = "/dev/ttyO1";
	int fd;
	unsigned char holdSpeed = 0;
	unsigned char holdDirection = 0;
	int rotateRegisterValue;
	unsigned char speedRegisterValue = 0;
	unsigned char check;
	unsigned char sendBuf[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
													// index 0: Start transmit
													// index 1: FROMBEAGLE = 0x01 = controle
													// index 2: is de PWM counter MSB voor de linker servo
													// index 3: is de PWM counter LSB voor de linker servo
													// index 4: is de PWM counter MSB voor de Rechter servo
													// index 5: is de PWM counter LSB voor de Rechter servo
													// index 6: is de snelheid waarde voor de DC Motor control
													// index 7: is rc5 commando
													// index 8: ETX einde transmit

	fd = ser_open(tty, 9600);

	// initialiseer de speed motor in een default aandrijf kracht
	// initialiseer de rotate motor in een default positie
	// initialiseer de besturen in default modus (Normal mode)
	givingState->automaticControl = OFF_STATE;
	givingState->hold_speed_direction = OFF_STATE;
	givingState->moveSpeed = 0;		// 0 procent (speed motor staat uit)
	givingState->rotateAngle = 50;	// 50 procent (midden positie of vooruit)

	while(1)
	{
		//---------Besturen modus-------------

		// Automatic control mode
		// heeft hoger proiriteit en moet als eerst getest wordt
		if (givingState->automaticControl == ON_STATE)
		{
			holdSpeed = 0;
			holdDirection = 0;

			// -----STILL TO DO----
			printf("Process in the Automatic mode\n");
			sleep(1);
		}
		// Hold speed/direction mode
		// Heeft hoger prioriteit boven de Normale mode en dus als tweede getest
		else if (givingState->hold_speed_direction == ON_STATE)
		{
			if (givingState->rotateAngle < 100 && givingState->moveSpeed < 100)
			{
				if (holdDirection < 50)
				{
					if ((givingState->rotateAngle < 50) && (givingState->rotateAngle < holdDirection))
					{
						holdDirection = givingState->rotateAngle;
					}
					else if (givingState->rotateAngle > 50)
					{
						holdDirection = givingState->rotateAngle;
					}
				}
				else
				{
					if ((givingState->rotateAngle > 50) && (givingState->rotateAngle > holdDirection))
					{
						holdDirection = givingState->rotateAngle;
					}
					else if (givingState->rotateAngle < 50)
					{
						holdDirection = givingState->rotateAngle;
					}
				}
				rotateRegisterValue = calculate_Rotate_Value(holdDirection);
				if (givingState->moveSpeed > holdSpeed)
				{
					holdSpeed = givingState->moveSpeed;
				}
				speedRegisterValue = calculate_Speed_Value(holdSpeed);
			}
			sleep(1);
			printf("Process in the Hold mode\n");
		}
		else
		{	// Normal mode
			holdSpeed = 0;
			holdDirection = 0;
			if (givingState->rotateAngle < 100 && givingState->moveSpeed < 100)
			{
				rotateRegisterValue = calculate_Rotate_Value(givingState->rotateAngle);
				speedRegisterValue = calculate_Speed_Value(givingState->moveSpeed);
			}
			sleep(1);
			//printf("Process in the Normal mode\n");
		}

		sendBuf[0] = STX;
		sendBuf[1] = FROMBEAGLE;
		sendBuf[2] = rotateRegisterValue;
		sendBuf[3] = (rotateRegisterValue >> 8);
		sendBuf[4] = rotateRegisterValue;
		sendBuf[5] = (rotateRegisterValue >> 8);
		sendBuf[6] = speedRegisterValue;
		sendBuf[7] = RC5_cmd;
		sendBuf[8] = ETX;

		check = ser_send_verify(fd, sendBuf);

		if (check == 0)
		{
			printf("Error: send, receive and verify failed.\n");
		}
	}

	ser_close(fd);

	return 0;
}

//Thread voor het uitlezen van srf02 sensor en output van afstand tot object elke 3 seconde
void* distanceReadHandler(void* arg)
{
	sem_t *semdes = SEM_FAILED;
	DataStructure *sensorsData;
	int file;
	int shm_fd;

	file = openI2C();
	ioctlI2C(file);
	char buf[5];
	int range;

	// creeer de data shared memory

	if ((shm_fd = shm_open(SHAREDATA, O_RDWR, 0777)) == -1)
	{
		perror("Error: cannot reopen shm");
		return 0;
	}

	// set the shared memory size
	if (ftruncate(shm_fd, DATASIZE) != 0)
	{
		perror("Error: cannot set memory size");
		return 0;
	}

	// map shared memory in address space
	if ((sensorsData = mmap(0, DATASIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0)) == MAP_FAILED)
	{
		perror("Error: cannot map memory");
		return 0;
	}

	// creer de semaphore voor de shared memory
	if ( (semdes = sem_open(SEM_SHAREDATA, 0)) == SEM_FAILED)
	{
			perror("Error: reopening the sem");
			return 0;
	}

	while(1)
	{
		buf[0] = 0x00;										//0x00 is het commando register van srf02
		buf[1] = 0x51;										//0x51 is het commando voor het uitlzen van srf02 in centimeters (0x50 in inches, 0x52 in ms)

		writeI2C(file, buf, 2);
		usleep(70000);										//wacht tot meting klaar is (meting duurt 65ms)

		buf[0] = 0x02;										//highbyte van afstand in register 0x02
		writeI2C(file, buf, 1);
		readI2C(file, buf, 1);
		range = buf[0] << 8;

		buf[0] = 0x03;										//lowbyte van afstand in register 0x03
		writeI2C(file, buf, 1);
		readI2C(file, buf, 1);

		range |= buf[0];									//range = high + low byte

		printf("Afstand : %d\n",range);

		sem_wait(semdes);

		sensorsData->distance = range;

		sem_post(semdes);

		sleep(1);

		//printf("Afstand %d\n", range);
	}

	//close
	//close sem
}

/**
 * Send the giving array of characters to the AVR microcontroller through a serial line
 *
 * @return value: returns 1 if the send, receive and verify is successful
 * 		: returns 0 if the send, receive and verify is not successful
 */
unsigned char ser_send_verify(int fd, unsigned char sendBuf[])
{
	unsigned char sendComplete = 0;
	const unsigned char TOTAL_SEND_TRY = 10;
	unsigned char sendCount = TOTAL_SEND_TRY;
	unsigned int index = 0;
	int bufferLen = 9;
	unsigned char recvBuf[] = {0, 0, 0, 0, 0, 0, 0, 0, 0 };

	while ((sendComplete == 0) && (sendCount > 0))
	{
		index = 0;
		sendComplete = 1;
		sendCount--;

		// verstuur de data
		ser_send(fd, sendBuf, bufferLen);

		// ontvang de verstuurde data
		ser_recv(fd, recvBuf, bufferLen);

		// check of de ontvangen data klopt met vertuurde data
		for (index = 0; index < bufferLen; index++)
		{
			if(recvBuf[index] != sendBuf[index])
			{
				if(index !=7 ) // RC5 Command byte
				{
					sendComplete = 0;
					sleep(0.1);
					tcflush(fd, TCIOFLUSH);
					break;
				}
			}
		}

		if(recvBuf[7] != 0x00)	// RC5 commando ontvangen ?!
		{
			RC5_cmd = recvBuf[7];
			printf("RC5 commando ontvangen: %x\n", recvBuf[7]);

			unsigned int priority = 1;

			CommandStructure sendData;
			mqd_t qd;

			// open a message queue named "contol_queue"
			if ((qd = mq_open(QUEUENAME, O_WRONLY)) == -1)
			{
				perror("Error: opening the queue from sendCommandTestHandler");
				return 0;
			}

			switch(RC5_cmd)
			{
				case hoverboard_up:
					sendData.command = LiftHoverboard;
					sendData.commandValue = 0x00;
					printf("Lift : %d\n", toggleLiftMotor);
				break;

			}

			RC5_cmd = 0x00;

			// stuurt de waarde naar de message queue
			if (mq_send(qd, (char *)&sendData, COMMANDSIZE, priority) == -1)
			{
				perror("Error: cannot send message");
			}

			// close message queue
			if (mq_close(qd) == -1)
			{
				perror("Error: closing queue");
			}
		}

		// reset de recvBuf, dus maak het gered voor de volgende keer
		for (index = 0; index < bufferLen; index++)
		{
			recvBuf[index] = 0;
		}
	}

	return sendComplete;
}

/**
 * Bereken de snelheid waarde tussen 0 en 255 met de aagegeven procent waarde
 */
unsigned char calculate_Speed_Value(unsigned char speedPercentage)
{
	const int SPEED_RESOLUTION = 255;

	return ((speedPercentage / 100.0) * SPEED_RESOLUTION);
}

/**
 * Bereken de pwm rotatie postie met de aagegeven procent waarde
 */
int calculate_Rotate_Value(unsigned char rotatePercentage)
{
	const int MINCOUNTERVALUE = 56100;
	const int ROTATE_RESOLUTION = 7300;

	return MINCOUNTERVALUE + (ROTATE_RESOLUTION *(rotatePercentage / 100.0));
}
