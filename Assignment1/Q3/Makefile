
obj-m = split.o
split-objs = file1.o file2.o

split.ko: file1.c file2.c
	make -C /lib/modules/$$(uname -r)/build M=$$(pwd) modules

clean:
	make -C /lib/modules/$$(uname -r)/build M=$$(pwd) clean

.PHONY: clean
