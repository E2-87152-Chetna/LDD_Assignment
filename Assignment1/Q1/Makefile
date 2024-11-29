
obj-m = hello.o


hello.ko: hello.c
	make -C /lib/modules/$$(uname -r)/build M=$$(pwd) modules

clean:
	make -C /lib/modules/$$(uname -r)/build M=$$(pwd) clean

.PHONY: clean
