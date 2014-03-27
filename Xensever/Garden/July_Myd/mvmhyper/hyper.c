#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
//#include <string.h>
//#include <xen/xenctrl.h>
//#include <xen/privcmd.h>

typedef struct privcmd_hypercall
{
	unsigned long long op;
	unsigned long long arg[5];
} privcmd_hypercall_t;

struct mmuext_op {
	unsigned int cmd;
	union {
	/* [UN]PIN_TABLE, NEW_BASEPTR, NEW_USER_BASEPTR */
			unsigned long mfn;
	/* INVLPG_LOCAL, INVLPG_ALL, SET_LDT */
			unsigned long linear_addr;
		} arg1;
		union {
			/* SET_LDT */
				unsigned int nr_ents;
			/* TLB_FLUSH_MULTI, INVLPG_MULTI */
				void * vcpumask;
	} arg2;
};
typedef struct mmuext_op mmuext_op_t;

int main(int argc, char *argv[])
{
    int fd, ret;

	mmuext_op_t op26;
	op26.cmd=2;
	op26.arg1.mfn=0xb9922;
	op26.arg1.linear_addr=0xb9922;
	op26.arg2.nr_ents=152828616;
	op26.arg2.vcpumask=0x91bfac8;
	unsigned long long abc;
     privcmd_hypercall_t hcall = {
         17,
         { 7, 0, &abc, 0, 0}
     };
     fd = open("/proc/xen/privcmd", O_RDWR);
     if (fd < 0) {
         perror("open");
         exit(1);
     } else
    printf("fd = %d\n", fd);
    ret = ioctl(fd, 0x305000, &hcall);
    printf("ret = 0x%x\n", ret);
	printf("abc = 0x%llx\n",abc);
	return 0;
}
