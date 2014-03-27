#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>
#include <linux/sched.h>

static int changehyper(char * filename, char __user *__user *argv, char __user *__user *envp, struct pt_regs * regs)
{
	printk("changehyper for %s from %s\n", filename, current->comm);
	jprobe_return();
	return 0;
}

static struct jprobe jphyper={
	.entry = (kprobe_opcode_t *) changehyper
};

static int mvmhyper_init(void)
{
	int ret;
	printk("mvmhyper init.\n");
	jphyper.kp.addr=0;
	jphyper.kp.symbol_name="do_mmu_update";
	//jphyper.kp.addr=(kprobe_opcode_t *) kallsyms_lookup_name("do_execve");
	/*
	if(jphyper.kp.addr == NULL)
	{
		printk("Error, can't find hyper address.\n");
		return -1;
	}
	*/
	if((ret=register_jprobe(&jphyper))<0)
	{
		printk("Error, register probe failed with return %d\n",ret);
		return -1;
	}
	printk("Planted probe at %p, handler addr %p\n", jphyper.kp.addr, jphyper.entry);
	return 0;
}

static void mvmhyper_exit(void)
{
	printk("mvmhyper exit.\n");
	unregister_jprobe(&jphyper);
}

module_init(mvmhyper_init);
module_exit(mvmhyper_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Little7");

