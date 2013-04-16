/*
 * I2CFunctions.c
 *
 *  Created on: Apr 15, 2013
 *      Author: student
 */

/**
*opent I2C bus  ( bus nummer moet nog meegegeven worden als parameter in functie, nu nog statisch)
*/
#include "I2CFunctions.h"

int openI2C(void){
	int file;
	char filename[40];
	sprintf(filename,"/dev/i2c-2");
	if((file = open(filename, O_RDWR))<0){
		perror("Failed to open te i2c bus");
		printf("Failed to open");
		exit(1);
	}
	return file;
}

/**
*geeft aan welk adres I2C apparaat is aangesloten(later variabel maken, nu nog statisch 0x70)
*/
void ioctlI2C(int file){
	int addr = 0x70;
	if(ioctl(file, I2C_SLAVE, addr) < 0){
		printf(" Failed to acquire bus acces and or talk to slave\n");
		exit(1);
	}
}

/**
* schrijft naar I2C
*/
void writeI2C(int file, char* buf,int bufSize){
	const char *buffer;
	if(write(file,buf,bufSize) !=bufSize)
	{
		printf("Failed to wrtie bu[0] to the i2c bus.\n");
		buffer = strerror(errno);
		printf(buffer);
		printf("\n");
		exit(1);
	}
}

/**
* leest I2C
*/
char* readI2C(int file,char* buf, int size){
	if ((read(file,buf,size)!=size))
	{
		printf(" eror on read");
		exit(1);
	}
	return buf;
}
