/*
 * serial.h
 *
 *  Created on: Feb 12, 2013
 *      Author: George Okeke
 */

#ifndef serial_h
#define serial_h

int ser_open(char * port, long baud);
void ser_close(int fd);
int ser_send(int fd, unsigned char * buf, size_t buflen);
int ser_recv(int fd, unsigned char * buf, size_t buflen);

#endif /* serial_h */
