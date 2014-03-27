#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>
#include <linux/sched.h>

#define hypernum 1

struct kprobe kphyper[hypernum];

int i=0;

int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	printk("Fortune: pre_handler: p->addr=0x%p\n",p->addr);
	return 0;
}

void handler_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags)
{
	printk("Fortune: post_handler: p->addr=0x%p\n",p->addr);
}

int handler_fault(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
	printk("Fortune: fault_handler:p->addr=0x%p\n",p->addr);
	return 0;
}

static int mvmhyper_init(void)
{
	printk("mvmhyper init.\n");
	for(i=0;i<hypernum;++i)
	{
		kphyper[i].pre_handler=handler_pre;
		kphyper[i].post_handler=handler_post;
		kphyper[i].fault_handler=handler_fault;
		//kphyper[i].addr=(kprobe_opcode_t *) (kallsyms_lookup_name("hypercall_page")+i*0x20);
		//kphyper[i].addr=(kprobe_opcode_t *) (0xc0401000 + (#(__HYPERVISOR_##mmuext_op) * 32));
		//kphyper[i].addr=(kprobe_opcode_t *) (0xc0401000+i*32);
		kphyper[i].symbol_name="direct_remap_pfn_range";
		kphyper[i].addr=0;
		register_kprobe(&kphyper[i]);
		if(kphyper[i].addr == NULL)
		{
			printk("Fortune: Error, can't find hyper address.\n");
			return 1;
		}
		printk("Fortune: Kprobe at %p\n",kphyper[i].addr);
		printk("Fortune: code is 0x%lx\n",*((long *)kphyper[i].addr));
		*((long *)kphyper[i].addr)=7;printk("Fortune: code is 0x%lx\n",*((long *)kphyper[i].addr));
	}
	return 0;
}

static void mvmhyper_exit(void)
{
	printk("mvmhyper exit.\n");
	for(i=0;i<hypernum;++i)
		unregister_kprobe(&kphyper[i]);
}

module_init(mvmhyper_init);
module_exit(mvmhyper_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Little7");

