#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>
#include <linux/sched.h>
#include <linux/ioctl.h>
#include <xen/privcmd.h>

#define hypernum 3

struct kprobe kphyper[hypernum];
struct kretprobe rphyper;
int i=0,time=0,res=0;

int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	++time;
	printk("Fortune: pre_handler: p->addr=0x%p\ttime %d\n",p->addr,time);
	return 0;
}

void handler_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags)
{
	++time;
	printk("Fortune: post_handler: p->addr=0x%p\ttime %d\n",p->addr,time);
}

int handler_fault(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
	printk("Fortune: fault_handler:p->addr=0x%p\n",p->addr);
	return 0;
}

static int handler_ret(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	//--time;
	printk("Fortune: ret_handler:%d\n",time);
	return 0;
}

static int handler_entry(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	time=0;
	printk("Fortune: entry_handler:%d\n",time);
	return 0;
}

static int mvmhyper_init(void)
{
	printk("mvmhyper init.\n");
	//printk("%x\n", IOCTL_PRIVCMD_HYPERCALL);
	for(i=0;i<hypernum;++i)
	{
		kphyper[i].pre_handler=handler_pre;
		kphyper[i].post_handler=handler_post;
		kphyper[i].fault_handler=handler_fault;
		//kphyper[i].addr=(kprobe_opcode_t *) (kallsyms_lookup_name("sys_ioctl"));
		kphyper[i].addr=(kprobe_opcode_t *) (kallsyms_lookup_name("do_execve")+(i+1)*1);
		//kphyper[i].addr=(kprobe_opcode_t *) (0x00080000+i*0x20);
		//kphyper.symbol_name="hypercall_page"; kallsyms_lookup_name can't done
		//kphyper.addr=0;
		register_kprobe(&kphyper[i]);
		if(kphyper[i].addr == NULL)
		{
			printk("Fortune: Error, can't find hyper address.\n");
			return 1;
		}
		printk("Fortune: Kprobe at %p\n",kphyper[i].addr);
		//printk("Fortune: code is 0x%lx\n",*((long *)kphyper[i].addr));
	}
	rphyper.handler=handler_ret;
	rphyper.entry_handler=handler_entry;
	rphyper.maxactive=20;
	rphyper.kp.addr=(kprobe_opcode_t *) (kallsyms_lookup_name("do_execve"));
	res=register_kretprobe(&rphyper);
	if(res < 0)
	{
		printk("Fortune: Error, can't find rphyper address.\n");
		return 1;
	}
	printk("Fortune: Kretprobe at %p\n",rphyper.kp.addr);
	return 0;
}

static void mvmhyper_exit(void)
{
	printk("mvmhyper exit.\n");
	for(i=0;i<hypernum;++i)
		unregister_kprobe(&kphyper[i]);
	unregister_kretprobe(&rphyper);
}

module_init(mvmhyper_init);
module_exit(mvmhyper_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Little7");

