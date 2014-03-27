#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>
#include <linux/sched.h>

#define hypernum 1

struct kretprobe rphyper[hypernum];

int i=0,res=0,count=0;

static int handler_ret(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	--count;
	printk("Fortune: ret_handler:%p\n",ri->addr);
	return 0;
}

static int handler_entry(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	++count;
	printk("Fortune: entry_handler:%p\n",ri->addr);
}

static int mvmhyper_init(void)
{
	printk("mvmhyper init.\n");
	for(i=0;i<hypernum;++i)
	{
		rphyper[i].handler=handler_ret;
		rphyper[i].entry_handler=handler_entry;
		rphyper[i].maxactive=20;
		rphyper[i].kp.addr=(kprobe_opcode_t *) (kallsyms_lookup_name("do_execve")+i*1);
		//kphyper[i].addr=(kprobe_opcode_t *) (0xc0401000 + (#(__HYPERVISOR_##mmuext_op) * 32));
		//kphyper[i].addr=(kprobe_opcode_t *) (0xc0401000+i*32);
		//kphyper[i].symbol_name="direct_remap_pfn_range";
		//kphyper[i].addr=0;
		res=register_kretprobe(&rphyper[i]);
		if(res < 0)
		{
			printk("Fortune: Error, can't find hyper address.\n");
			return 1;
		}
		printk("Fortune: Kretprobe at %p\n",rphyper[i].addr);
		//printk("Fortune: code is 0x%lx\n",*((long *)kphyper[i].addr));
		//*((long *)kphyper[i].addr)=7;printk("Fortune: code is 0x%lx\n",*((long *)kphyper[i].addr));
	}
	return 0;
}

static void mvmhyper_exit(void)
{
	printk("mvmhyper exit.\n");
	for(i=0;i<hypernum;++i)
		unregister_kretprobe(&rphyper[i]);
}

module_init(mvmhyper_init);
module_exit(mvmhyper_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Little7");

