#include <linux/kprobes.h>
#include <linux/kallsyms.h>
#include <linux/module.h>
#include <linux/uaccess.h>

static struct jprobe my_probe;

typedef struct privcmd_hypercall
{
	__u64 op;
	__u64 arg[5];
}privcmd_hypercall_t;

void get_random_bytes(void *buf, int nbytes);

static int test_count;

asmlinkage long my_handler(unsigned int fd, unsigned int cmd, unsigned long arg)
{
	privcmd_hypercall_t hypercall;
	void __user *udata = (void __user *)arg;
	unsigned long rand_num;
	__u64 orig_op, evtchn_op;// orig_data=0, new_data;
	
	//printk("Before sys_ioctl: cmd = 0x%x\n",cmd);
	if(cmd == 0x305000) //IOCTL_PRIVCMD_HYPERCALL
	{
		//Read original hypercall arg
		if(copy_from_user(&hypercall, udata, sizeof(hypercall)))
		{
			printk("Before sys_ioctl: copy_from_user error\n");
			jprobe_return();
		}
		orig_op = hypercall.op;

		//printk("Before sys_ioctl: cmd = 0x%x\top=%lld\t\n",cmd, orig_op);
		if(orig_op == 32)
		{
			//if(test_count == 9) jprobe_return();
			
			get_random_bytes(&rand_num, sizeof(unsigned long));
			rand_num = rand_num % 11;
			//hypercall.op = rand_num;
			evtchn_op = hypercall.arg[0];
			//hypercall.arg[0] = (evtchn_op == rand_num) ? rand_num -1 : rand_num;
			hypercall.arg[1] = 0;

			if(copy_to_user(udata, &hypercall, sizeof(hypercall)))
			{
				printk("Before sys_ioctl: copy_to_user error\n");
				jprobe_return();
			}

			printk("Before sys_ioctl: cmd = 0x%x\top=%lld\tevtchn_op=%lld\n",cmd, orig_op, evtchn_op);
			test_count++;
		}
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
	printk("Hypercall Evtchn FI kernel module: init\n");
	return 0;
}

static void __exit my_exit(void)
{
	unregister_jprobe(&my_probe);
	printk("Hypercall Evtchn FI kernel module: exit\n");
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("HIT CS HDMC team");
MODULE_DESCRIPTION("Hypercall FI Module.");
