PWD := $(shell pwd)
obj-m += netlink.o
KDIR := /home/and/sources/linux-5.3.13
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

all:
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C ${KDIR} M=$(PWD) modules
	ARCH=arm arm-linux-gnueabihf-gcc -o nluser nluser.c

clean:
	make -C ${KDIR} M=$(PWD) clean
	rm nluser
