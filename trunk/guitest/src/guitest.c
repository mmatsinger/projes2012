/*
 ============================================================================
 Name        : guitest.c
 Author      : Roy Gluc
 Version     : 0.2
 Copyright   : GNU Public License
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "Hoverboard_Defines.h"
#include <mqueue.h>
#include <string.h>


int main(void) {
	char *data; //raw webinput
	char webcommand[10]; //clean webinput
	char *token; //strtok token
	char command[10]; // clean command
	char commandVal[5]; // clean commandVal
	mqd_t mq; // Message Queue
	CommandStructure sendCom;

    if ((mq = mq_open(QUEUENAME, O_WRONLY)) == -1) {
            perror("Error: Can't open queue");
    }


	// Print HTML header
	printf("%s%c%c\n",
	"Content-Type:text/html;charset=iso-8859-1",13,10);

	// Get command + command value
	data = getenv("QUERY_STRING");
	strcpy(webcommand, data);
	token = strtok(webcommand,"=");
	strcpy(command,token);
	token = strtok(NULL,"=");
	strcpy(commandVal, token);

	//Debug alert
	printf("Received Command: %s with Value: %s",command,commandVal);

	//Toggle commands als lift/hold/autoPilot moeten op de daemon bijgehouden worden?

	if (strcmp(command,"lift") == 0){
		printf("Received Command: %s with Value: %s",command,commandVal);
		sendCom.command = LiftHoverboard;
		sendCom.commandValue = 0;
		mq_send(mq, (char *)&sendCom, COMMANDSIZE, 0);
	}
	else if (strcmp(command,"hold") == 0){
		printf("Received Command: %s with Value: %s",command,commandVal);
		sendCom.command = Hold_Speed_Direction;
		sendCom.commandValue = 0;
		mq_send(mq, (char *)&sendCom, COMMANDSIZE, 0);
	}
	else if (strcmp(command,"autoPilot") == 0){
		printf("Received Command: %s with Value: %s",command,commandVal);
		sendCom.command = ActivateAutomaticControl;
		sendCom.commandValue = 0;
		mq_send(mq, (char *)&sendCom, COMMANDSIZE, 0);
	}
	else if (strcmp(command,"move") == 0){
		printf("Received Command: %s with Value: %s",command,commandVal);
		sendCom.command = move;
		sendCom.commandValue = atoi(commandVal);
		mq_send(mq, (char *)&sendCom, COMMANDSIZE, 0);
	}
	else if (strcmp(command,"rotate") == 0){
		printf("Received Command: %s with Value: %s",command,commandVal);
		sendCom.command = rotate;
		sendCom.commandValue = atoi(commandVal);
		mq_send(mq, (char *)&sendCom, COMMANDSIZE, 0);
	}
	else{
		printf("Command not recoqnized");
	}

	mq_close(mq);
	return 0;
}
