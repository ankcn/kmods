PWD := $(shell pwd)
obj-m += sysfs.o
KDIR := /home/and/sources/linux-5.3.13
ccflags-y := -std=gnu99 -Wno-declaration-after-statement


all:
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C ${KDIR} M=$(PWD) modules

clean:
	make -C ${KDIR} M=$(PWD) clean


