
obj-m = bbb_gpio.o
ksrcdir = /home/sunbeam/linux/linux

bbb_gpio.ko: bbb_gpio.c
	make -C $(ksrcdir) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- M=$$(pwd) modules

clean:
	make -C $(ksrcdir) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- M=$$(pwd) clean

.PHONY: clean

