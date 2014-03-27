obj-m := page_entry_fi.o
all:
	make -C /lib/modules/`uname -r`/build M=`pwd` modules
clean:
	make -C /lib/modules/`uname -r`/build M=`pwd` clean
	rm -rf *.o *.ko *.mod.c Module.markers .*.cmd .tmp_versions *.order *.symvers

