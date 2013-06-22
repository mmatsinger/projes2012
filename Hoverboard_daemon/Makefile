CC = /opt/cross_arm/bb_toolchain/arm-unknown-linux-uclibcgnueabi/bin/arm-unknown-linux-uclibcgnueabi-gcc
CFLAGS = -O2 -Wall -Werror
LDFLAGS = -lrt -lpthread
HEADERS = Hoverboard_Defines.h
SOURCES = daemon.c
OBJECTS = $(SOURCES:.c=.o)
TARGETS = Hoverboard_daemon

.PHONY: all clean

all: $(TARGETS)

$(TARGETS) : $(OBJECTS) $(HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

clean:
	@rm -rf $(TARGETS) $(OBJECTS)
