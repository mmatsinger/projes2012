################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../GPIO_Functions.c \
../I2CFunctions.c \
../daemon.c \
../ser_posix.c 

OBJS += \
./GPIO_Functions.o \
./I2CFunctions.o \
./daemon.o \
./ser_posix.o 

C_DEPS += \
./GPIO_Functions.d \
./I2CFunctions.d \
./daemon.d \
./ser_posix.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	/opt/eldk-5.3/armv7a/sysroots/i686-eldk-linux/usr/bin/armv7a-vfp-neon-linux-gnueabi/arm-linux-gnueabi-gcc -I/opt/eldk-5.3/armv7a/sysroots/armv7a-vfp-neon-linux-gnueabi/usr/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


