PWD := $(shell pwd)
obj-m += chardev.o
KDIR := /home/and/sources/linux-5.3.13

all:
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C ${KDIR} M=$(PWD) modules
	ARCH=arm arm-linux-gnueabihf-gcc -o ioctl ioctl.c

clean:
	make -C ${KDIR} M=$(PWD) clean
	rm ioctl
