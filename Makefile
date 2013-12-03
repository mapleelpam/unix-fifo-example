
CC=/opt/altera-linux/linaro/gcc-linaro-arm-linux-gnueabihf-4.7-2012.11-20121123_linux/bin/arm-linux-gnueabihf-gcc

CFLAGS=-Wall -march=armv7-a -mtune=cortex-a9 -mfpu=neon -ftree-vectorize

TARGET = fifo_cli fifo_srv nw

all: ${TARGET}


clean:
	rm -rf ${TARGET}

.PHONY: all clean
