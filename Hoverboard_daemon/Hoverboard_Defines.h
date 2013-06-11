/*
 * Hoverboard_Defines.h
 *
 *  Created on: Jan 4, 2013
 *      Author: George Okeke
 * Edited by Roy
 */

#ifndef HOVERBOARD_DEFINES_H_
#define HOVERBOARD_DEFINES_H_

#include <stdio.h>
#include <stdlib.h>

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

typedef enum
{
  hoverboard_up 	= 0x10,
  hoverboard_down 	= 0x11
} RC5_commando;

// vastgelegde commando waarde
#define LiftHoverboard 0x01
#define Hold_Speed_Direction 0x02
#define ActivateAutomaticControl 0x03
#define move 0x04
#define rotate 0x05
#define termninate_prog 0x06

#define ON_STATE 0x01
#define OFF_STATE 0x00
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF 64
#define DIROUT 1	// om de gpio als output in te stellen
#define DIRIN 0		// om de gpio als input in te stellen

#define TRUE 1
#define FALSE 0

#define FROMBEAGLE 	0x01
#define STX 		0x02
#define ETX 		0x03

static const char QUEUENAME[] = "/queue";	// De naam van de message queue
static const char SHAREDATA[] = "shmData";		// De naam van de shared memory
static const char SEM_SHAREDATA[] = "dataSem";	// De naam van de semaphore bij het gebruik van de data shared memory

#define COMMANDSIZE sizeof(CommandStructure)
#define DATASIZE sizeof(DataStructure)

#endif /* HOVERBOARD_DEFINES_H_ */
