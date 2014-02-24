obj-m := evtchn_fi.o
all:
	make -C /lib/modules/`uname -r`/build M=`pwd` modules
install:
	insmod hypercall_fi.ko
remove:
	rmmod hypercall_fi
clean:
	make -C /lib/modules/`uname -r`/build M=`pwd` clean
	rm -f modules.order Module.symvers Module.markers
