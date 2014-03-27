#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <xenctrl.h>
#include <xen/linux/privcmd.h>
#include <xen/linux/evtchn.h>
int main(int argc, char *argv[])
{
	/*
    int fd, ret;
    char * message;
    if (argc != 2) {
        printf("please put one parameter!/n");
        return -1;
    }
    message = (char *) malloc(sizeof(char) * (strlen(argv[1])+1));
    strcpy(message, argv[1]);
    privcmd_hypercall_t hcall = {
        __HYPERVISOR_print_string,
        {message, 0, 0, 0, 0}
    };
    fd = open("/proc/xen/privcmd", O_RDWR);
    if (fd < 0) {
        perror("open");
        exit(1);
    } else
        printf("fd = %d/n", fd);
    ret = ioctl(fd, IOCTL_PRIVCMD_HYPERCALL, &hcall);
    printf("ret = %d/n", ret);
	*/
	printf("0x%x\n", IOCTL_PRIVCMD_HYPERCALL);
	printf("Fortune: 0x%x\n",HYPERCALL_PHYSICAL_ADDRESS);
	printf("IOCTL_EVTCHN_BIND_VIRQ = 0x%x\n", IOCTL_EVTCHN_BIND_VIRQ);
	printf("IOCTL_EVTCHN_BIND_INTERDOMAIN = 0x%x\n", IOCTL_EVTCHN_BIND_INTERDOMAIN);
	printf("IOCTL_EVTCHN_BIND_UNBOUND_PORT = 0x%x\n", IOCTL_EVTCHN_BIND_UNBOUND_PORT);
	printf("IOCTL_EVTCHN_UNBIND = 0x%x\n", IOCTL_EVTCHN_UNBIND);
	printf("IOCTL_EVTCHN_NOTIFY = 0x%x\n", IOCTL_EVTCHN_NOTIFY);
	printf("IOCTL_EVTCHN_RESET = 0x%x\n", IOCTL_EVTCHN_RESET);
	return 0;
}
