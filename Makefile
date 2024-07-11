arch ?= x86
modname ?= chrdev_ioctl
appname ?= main

# 当前路径
currentDir := $(shell pwd)

# linux内核源码的路径
ifeq ($(arch),arm)
	LinuxDir := /home/linux/linux-5.10.61
	CROSS_COMPILE ?= arm-linux-gnueabihf-
# KBUILD_EXTRA_SYMBOLS := /home/linux/DC23121/linux_drivers/day02/01export_symbol/demoA/Module.symvers
else
	LinuxDir := /lib/modules/$(shell uname -r)/build
	CROSS_COMPILE ?=
# KBUILD_EXTRA_SYMBOLS := /home/linux/DC23121/linux_drivers/day02/01export_symbol/demoA/Module.symvers
endif

CC := $(CROSS_COMPILE)gcc

all:
	@# 编译模块的驱动程序
	make -C $(LinuxDir) M=$(currentDir) modules 
	@# 编译应用程序
	$(CC) $(appname).c -o $(appname)

clean:
	make -C $(LinuxDir) M=$(currentDir) clean 
	rm $(appname)
# rm -rf ~/rootfs/$(modname)_driver
install:
	mkdir ~/rootfs/$(modname)_driver
	cp *.ko $(appname) ~/rootfs/$(modname)_driver

help:
	@echo "build: 编译驱动程序和应用程序"
	@echo 'make arch=<arm|x86> modname=<divers file name> appname=<app file name> all'
	@echo "install: 拷贝驱动的.ko文件和应用程序到开发板的文件系统中"
	@echo 'make appname=<app file name> install'
	@echo "clean : 清除工程中的中间文件"
	@echo 'make clean'

obj-m += $(modname).o
