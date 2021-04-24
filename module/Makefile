obj-m += lkm_minifs.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

test:
	-sudo rmmod lkm_minifs
	sudo dmesg -C
	sudo insmod lkm_minifs.ko
	dmesg
