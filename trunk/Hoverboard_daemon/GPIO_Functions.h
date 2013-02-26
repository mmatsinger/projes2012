/*
 * funtionsHeader.h
 *
 *  Created on: Feb 12, 2013
 *      Author: George Okeke
 */

#ifndef GPIO_FUNCTIONS_H_
#define GPIO_FUNCTIONS_H_

#include <stdio.h>
#include <stdlib.h>

//------GPIO functions header------------------
int gpio_export(unsigned int gpio);
int gpio_unexport(unsigned int gpio);
int gpio_set_dir(unsigned int gpio, unsigned int out_flag);
int gpio_set_value(unsigned int gpio, unsigned int value);
int gpio_get_value(unsigned int gpio, unsigned int *value);
int gpio_set_edge(unsigned int gpio, char *edge);
int gpio_fd_open(unsigned int gpio);
int gpio_fd_close(int fd);
//------------------------------------------

#endif /* GPIO_FUNCTIONS_H_ */
