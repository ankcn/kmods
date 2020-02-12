PWD := $(shell pwd)
obj-m += devfile.o
KDIR := /home/and/sources/linux-5.3.13

all:
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C ${KDIR} M=$(PWD) modules

clean:
	make -C ${KDIR} M=$(PWD) clean


