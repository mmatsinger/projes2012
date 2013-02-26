/*
 * serial_posix.c
 *
 *  Created on: Feb 12, 2013
 *      Author: George Okeke
 */


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include "serial.h"

long serial_recv_timeout = 5000; /* ms */
static char *progname = "SER_POSIX";

struct baud_mapping {
  long baud;
  speed_t speed;
};

/* There are a lot more baud rates we could handle, but what's the point? */
static struct baud_mapping baud_lookup_table [] = {
  { 1200,   B1200 },
  { 2400,   B2400 },
  { 4800,   B4800 },
  { 9600,   B9600 },
  { 19200,  B19200 },
  { 38400,  B38400 },
  { 57600,  B57600 },
  { 115200, B115200 },
  { 230400, B230400 },
  { 0,      0 }                 /* Terminator. */
};

static struct termios original_termios;
static int saved_original_termios;

static speed_t serial_baud_lookup(long baud)
{
  struct baud_mapping *map = baud_lookup_table;

  while (map->baud) {
    if (map->baud == baud)
      return map->speed;
    map++;
  }

  return baud;
}

static int ser_setspeed(int fd, long baud)
{
  int rc;
  struct termios termios;
  speed_t speed = serial_baud_lookup (baud);
  
  if (!isatty(fd))
    return -ENOTTY;
  
  /*
   * initialize terminal modes
   */
  rc = tcgetattr(fd, &termios);
  if (rc < 0) {
    return -errno;
  }

  /*
   * copy termios for ser_close if we haven't already
   */
  if (! saved_original_termios++) {
    original_termios = termios;
  }

  termios.c_iflag = IGNBRK;
  termios.c_oflag = 0;
  termios.c_lflag = 0;
  termios.c_cflag = (CS8 | CREAD | CLOCAL);
  termios.c_cc[VMIN]  = 1;
  termios.c_cc[VTIME] = 0;

  cfsetospeed(&termios, speed);
  cfsetispeed(&termios, speed);

  rc = tcsetattr(fd, TCSANOW, &termios);
  if (rc < 0) {
    return -errno;
  }

  /*
   * Everything is now set up for a local line without modem control
   * or flow control, so clear O_NONBLOCK again.
   */
  rc = fcntl(fd, F_GETFL, 0);
  if (rc != -1)
    fcntl(fd, F_SETFL, rc & ~O_NONBLOCK);

  return 0;
}

int ser_open(char * port, long baud)
{
  int rc;
  int fd;

  printf("baud value: %ld\n", baud); 
  /*
   * open the serial port
   */
  fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd < 0) {
    fprintf(stderr, "%s: ser_open(): can't open device \"%s\": %s\n",
            progname, port, strerror(errno));
    exit(1);
  }

  /*
   * set serial line attributes
   */
  rc = ser_setspeed(fd, baud);
  if (rc) {
    fprintf(stderr, 
            "%s: ser_open(): can't set attributes for device \"%s\": %s\n",
            progname, port, strerror(-rc));
    exit(1);
  }

  tcflush(fd, TCIOFLUSH);

  return fd;
}


void ser_close(int fd)
{
  /*
   * restore original termios settings from ser_open
   */
  if (saved_original_termios) {
    int rc = tcsetattr(fd, TCSANOW | TCSADRAIN, &original_termios);
    if (rc) {
      fprintf(stderr, 
              "%s: ser_close(): can't reset attributes for device: %s\n",
              progname, strerror(errno));
    }
    saved_original_termios = 0;
  }

  close(fd);
}


int ser_send(int fd, unsigned char * buf, size_t buflen)
{
  struct timeval timeout, to2;
  int rc;
  unsigned char * p = buf;
  size_t len = buflen;

  if (!len)
    return 0;

  timeout.tv_sec = 0;
  timeout.tv_usec = 500000;
  to2 = timeout;

  while (len) {
    rc = write(fd, p, len);
    if (rc < 0) {
      fprintf(stderr, "%s: ser_send(): write error: %s\n",
              progname, strerror(errno));
      exit(1);
    }
    p += rc;
    len -= rc;
  }

  return 0;
}


int ser_recv(int fd, unsigned char * buf, size_t buflen)
{
  struct timeval timeout, to2;
  fd_set rfds;
  int nfds;
  int rc;
  unsigned char * p = buf;
  size_t len = 0;

  timeout.tv_sec  = serial_recv_timeout / 1000L;
  timeout.tv_usec = (serial_recv_timeout % 1000L) * 1000;
  to2 = timeout;

  while (len < buflen) {
  reselect:
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    nfds = select(fd + 1, &rfds, NULL, NULL, &to2);
    if (nfds == 0) {
      return -1;
    }
    else if (nfds == -1) {
      if (errno == EINTR || errno == EAGAIN) {
        goto reselect;
      }
      else {
        fprintf(stderr, "%s: ser_recv(): select(): %s\n",
                progname, strerror(errno));
        exit(1);
      }
    }

    rc = read(fd, p, buflen);
    if (rc < 0) {
      fprintf(stderr, "%s: ser_recv(): read error: %s\n",
              progname, strerror(errno));
      exit(1);
    }
    p += rc;
    len += rc;
  }

  return 0;
}



