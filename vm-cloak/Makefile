obj-m += vader.o
vader-objs := vm-cloak.o spoofer.o hooks.o

PWD := $(CURDIR)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -o cpuid_test cpuid_test.c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm cpuid_test

