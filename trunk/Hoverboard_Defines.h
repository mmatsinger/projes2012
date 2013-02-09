/*
 * Hoverboard_Defines.h
 *
 *  Created on: Jan 4, 2013
 *      Author: George
 */

#ifndef HOVERBOARD_DEFINES_H_
#define HOVERBOARD_DEFINES_H_

// This structure defines the structure of the data received
// from or send to the message queue
typedef struct
{
	unsigned char command;		// Bevat de commando
	unsigned char commandValue;	// Bij somige commando hoort een commando waarde (namelijk bij move en rotate commando).
}CommandStructure;

// Bevat de data structuur die gebruik wordt met de data shared memory
typedef struct
{
	float speed;
	float direction;
	float distance;
}DataStructure;

typedef struct
{
	unsigned char hold_speed_direction;
	unsigned char automaticControl;
	unsigned char moveSpeed;
	unsigned char rotateAngle;
}ControlState;

// vastgelegde commando waarde
#define LiftHoverboard 0x01
#define Hold_Speed_Direction 0x02
#define ActivateAutomaticControl 0x03
#define move 0x04
#define rotate 0x05

#define ON_STATE 0x01
#define OFF_STATE 0x00

const char QUEUENAME[] = "/queue";	// De naam van de message queue
const char SHAREDATA[] = "shmData";		// De naam van de shared memory
const char SEM_SHAREDATA[] = "dataSem";	// De naam van de semaphore bij het gebruik van de data shared memory

#define COMMANDSIZE sizeof(CommandStructure)
#define DATASIZE sizeof(DataStructure)

#endif /* HOVERBOARD_DEFINES_H_ */
