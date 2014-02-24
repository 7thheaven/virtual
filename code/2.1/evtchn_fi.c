#include <linux/kprobes.h>
#include <linux/kallsyms.h>
#include <linux/uaccess.h>
#include <linux/module.h>

static struct jprobe my_probe;

typedef struct privcmd_hypercall
{
	__u64 op;
	__u64 arg[5];
}privcmd_hypercall_t;

typedef struct ioctl_evtchn_notify
{
	unsigned int port;
}ioctl_evtchn_notify_t;

void get_random_bytes(void *buf, int nbytes);

static int test_count;

asmlinkage long my_handler(unsigned int fd, unsigned int cmd, unsigned long arg)
{
	ioctl_evtchn_notify_t evtchn;
	void __user *udata = (void __user *)arg;
	unsigned long rand_num;
	unsigned int port;
	//printk("Before sys_ioctl, cmd=0x%x\n", cmd);
	if(cmd == 0x44504) //IOCTL_EVTCHN_NOTIFY
	{
		//Read original hypercall arg
		if(copy_from_user(&evtchn, udata, sizeof(evtchn)))
		{
			printk("Before sys_ioctl: copy_from_user error\n");
			jprobe_return();
		}
		port = evtchn.port;

		//if(test_count == 10) jprobe_return();

		get_random_bytes(&rand_num, sizeof(unsigned long));
		rand_num = rand_num % 100;
		evtchn.port = rand_num;

		printk("Before sys_ioctl, cmd=0x%x\tport=%u\tfault_port=%u\n", cmd, port, evtchn.port);

		if(copy_to_user(udata, &evtchn, sizeof(evtchn)))
		{
			printk("Before sys_ioctl: copy_to_user error\n");
			jprobe_return();
		}
		test_count++;
	}
	jprobe_return();
}
static int register_hypercall_fi(void)
{
	test_count = 0;
	my_probe.kp.addr = (kprobe_opcode_t *) 0xc01d2d20; //sys_ioctl
	my_probe.entry = (kprobe_opcode_t *)my_handler;
	register_jprobe(&my_probe);
	return 0;
}

static int __init my_init(void)
{
	register_hypercall_fi();
	printk("Hypercall FI kernel module: init\n");
	return 0;
}

static void __exit my_exit(void)
{
	unregister_jprobe(&my_probe);
	printk("Hypercall FI kernel module: exit\n");
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("HIT CS HDMC team");
MODULE_DESCRIPTION("Hypercall FI Module.");
