#include <linux/init.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>
#include <linux/uaccess.h>
#include <xen/balloon.h>
#include <xen/blkif.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/blkdev.h>
#include <linux/vmalloc.h>
#include <linux/wait.h>
#include <asm/io.h>
#include <asm/setup.h>
#include <asm/pgalloc.h>
#include <xen/evtchn.h>
#include <asm/hypervisor.h>
#include <xen/gnttab.h>
#include <xen/driver_util.h>
#include <xen/xenbus.h>
#define pmd_val_ma(v) (v).pmd

static mmu_update_t last_value;
static int first_time = 1, count = 0;

void get_random_bytes(void *buf, int nbytes);

void my_xen_l3_entry_update(pmd_t *ptr, pmd_t val)
{
	mmu_update_t u;
//	unsigned long rand_num;
	printk("Page Entry FI module: l3_update\tcurrent_comm=%s\n", current->comm);
	if(current->comm[0] == 'x' &&
	   current->comm[1] == 'e' &&
	   current->comm[2] == 'n')
	{
		u.ptr = virt_to_machine(ptr);
		u.val = pmd_val_ma(val);
		if(!first_time)
		{
			//get_random_bytes(&rand_num, sizeof(unsigned long));
			//rand_num = rand_num % last_value.val;
			printk("Page Entry FI module: l2_update_erro\tval=0x%llx\tfault_val=0x%lx\tcurrent_comm=%s\n", last_value.val, 0,current->comm);
			last_value.val = ~(last_value.val);
			BUG_ON(HYPERVISOR_mmu_update(&last_value, 1, NULL, DOMID_SELF) < 0);
		}
		last_value.ptr = u.ptr;
		last_value.val = u.val;
		first_time = 0;
		count++;
	}
	jprobe_return();
}

static struct jprobe jp;

static int mvmhyper_init(void)
{
	int ret;
	printk("Page Entry FI module: init.\n");
	jp.entry = (kprobe_opcode_t *) my_xen_l3_entry_update;
	jp.kp.symbol_name="xen_l3_entry_update";
	jp.kp.addr=0;
	first_time = 1;
	count = 0;
	if((ret=register_jprobe(&jp))<0)
	{   
		printk("Page Entry FI module: Error, register probe failed with return %d\n", ret);
		return -1;
	}
	if(jp.kp.addr == NULL)
	{
		printk("Page Entry FI module: Error, can't find address.\n");
		return -1;
	}
	return 0;
}

static void mvmhyper_exit(void)
{
	unregister_jprobe(&jp);
	printk("Page Entry FI module exit.\n");
}

module_init(mvmhyper_init);
module_exit(mvmhyper_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("FG");
