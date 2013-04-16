/*
 * I2CFunctions.h
 *
 *  Created on: Apr 15, 2013
 *      Author: student
 */

#ifndef I2CFUNCTIONS_H_
#define I2CFUNCTIONS_H_

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
#include <linux/i2c-dev.h>

int openI2C(void);
void ioctlI2C(int file);
void writeI2C(int file, char* buf,int bufSize);
char* readI2C(int file,char* buf, int size);

#endif /* I2CFUNCTIONS_H_ */
