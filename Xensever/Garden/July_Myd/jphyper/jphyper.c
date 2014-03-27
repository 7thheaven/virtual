#include <linux/init.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/random.h>
#include <linux/time.h>
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

#define jpnum 1
#define PERMISSION	0644
#define MAX_LINE 256

struct proc_dir_entry *dir = NULL;
struct proc_dir_entry *proc_aim = NULL;
struct proc_dir_entry *proc_fault = NULL;
struct proc_dir_entry *proc_time = NULL;
struct proc_dir_entry *proc_id = NULL;
struct proc_dir_entry *proc_signal = NULL;

int i=0,ioctlcount=0,last=-1,time=0,aim=-1,fault=-1,id=-1,signal=0,flag=0,leave=0,idtemp;
u64 rando;
char temp[MAX_LINE],date[MAX_LINE];
mmu_update_t tta,ttb;
mmu_update_t mmuupdateop;
mmuext_op_t mmuextop;

void getrando(int n)
{
	get_random_bytes(&rando, sizeof(u64));
	rando = (((int)rando)%n+n)%n;
}

void my_xen_l2_entry_update(pmd_t *ptr, pmd_t val)
{
	if(aim!=2 || signal==0) jprobe_return();
	if(leave==1) {BUG_ON(HYPERVISOR_mmu_update(&mmuupdateop, 1, NULL, DOMID_SELF) < 0);leave=0;}
	//if((strcmp(current->comm,"xm")==0)||(strcmp(current->comm,"xend")==0)) printk("Fortune: %s\n",current->comm); else jprobe_return();
	if(time>0) --time; else {signal=0;printk("Done.\n");jprobe_return();}
	printk("Fortune: xen_l2_entry_update from %s\n",current->comm);
	printk("Fortune: pmd=0x%llx\tval=0x%llx\n",ptr->pmd,val.pmd);
	printk("Fortune: ptr real=0x%llx\n",virt_to_machine(ptr));
	mmuupdateop.ptr = virt_to_machine(ptr);
	mmuupdateop.val = pmd_val_ma(val);
	if(fault==0)
	{
		getrando(64);
		mmuupdateop.ptr ^= (1 << rando);
		printk("Fortune: change ptr to 0x%llx\n",mmuupdateop.ptr);
		leave=1;
	}
	if(fault==1)
	{
		getrando(64);
		ptr->pmd ^= (1 << rando);
		printk("Fortune: change pmd to 0x%llx\n",ptr->pmd);
		//not need to leave=1
	}
	if(fault==2)
	{
		getrando(64);
		//mmuupdateop.val ^= (1 << rando);
		mmuupdateop.val=0;
		printk("Fortune: change val to 0x%llx\n",mmuupdateop.val);
		leave=1;
	}
	jprobe_return();
}

int proc_read_aim(char * page,char **start, off_t off, int count, int * eof,void * data)
{
	int iLen;
	iLen = sprintf(page, "%d", aim);
	return iLen;
}

int proc_write_aim(struct file *file,const char *buffer,unsigned long count,void * data)
{
	int iRet;
	if(count <= 0) { return -1; }
	memset(temp, '\0', sizeof(temp));
	iRet = copy_from_user(temp, buffer, count);
	if(iRet) { return -1; }
	iRet = sscanf(temp,"%d",&aim);
	if(iRet != 1) { return -1; }
	printk("Fortune: write aim:%d\n",aim);
	return count;
}

int proc_read_fault(char * page,char **start, off_t off, int count, int * eof,void * data)
{
		int iLen;
		iLen = sprintf(page, "%d", fault);
		return iLen;
}

int proc_write_fault(struct file *file,const char *buffer,unsigned long count,void * data)
{
		int iRet;
		if(count <= 0) { return -1; }
		memset(temp, '\0', sizeof(temp));
		iRet = copy_from_user(temp, buffer, count);
		if(iRet) { return -1; }
		iRet = sscanf(temp,"%d",&fault);
		if(iRet != 1) { return -1; }
		printk("Fortune: write fault:%d\n",fault);
		return count;
}

int proc_read_time(char * page,char **start, off_t off, int count, int * eof,void * data)
{
	int iLen;
	iLen = sprintf(page, "%d", time);
	return iLen;
}

int proc_write_time(struct file *file,const char *buffer,unsigned long count,void * data)
{
	int iRet;
	if(count <= 0) { return -1; }
	memset(temp, '\0', sizeof(temp));
	iRet = copy_from_user(temp, buffer, count);
	if(iRet) { return -1; }
	iRet = sscanf(temp,"%d",&time);
	if(iRet != 1) { return -1; }
	printk("Fortune: write time:%d\n",time);
	return count;
}

int proc_read_id(char * page,char **start, off_t off, int count, int * eof,void * data)
{
	int iLen;
	iLen = sprintf(page, "%d", id);
	return iLen;
}

int proc_write_id(struct file *file,const char *buffer,unsigned long count,void * data)
{
	int iRet;
	if(count <= 0) { return -1; }
	memset(temp, '\0', sizeof(temp));
	iRet = copy_from_user(temp, buffer, count);
	if(iRet) { return -1; }
	iRet = sscanf(temp,"%d",&id);
	if(iRet != 1) { return -1; }
	printk("Fortune: write id:%d\n",id);
	return count;
}

int proc_read_signal(char * page,char **start, off_t off, int count, int * eof,void * data)
{
	int iLen;
	iLen = sprintf(page, "%d", signal);
	return iLen;
}

int proc_write_signal(struct file *file,const char *buffer,unsigned long count,void * data)
{
	int iRet;
	if(count <= 0) { return -1; }
	memset(temp, '\0', sizeof(temp));
	iRet = copy_from_user(temp, buffer, count);
	if(iRet) { return -1; }
	iRet = sscanf(temp,"%d",&signal);
	if(iRet != 1) { return -1; }
	printk("Fortune: write signal:%d\n",signal);
	return count;
}

static struct jprobe jphyper[jpnum];

static int mvmhyper_init(void)
{
	int ret;
	printk("mvmhyper init.\n");

	//jphyper[0].entry = (kprobe_opcode_t *) my_sys_ioctl;
	//jphyper[1].entry = (kprobe_opcode_t *) my_xen_pgd_pin;
	jphyper[0].entry = (kprobe_opcode_t *) my_xen_l2_entry_update;
	//jphyper[3].entry = (kprobe_opcode_t *) my___direct_remap_pfn_range;
	//jphyper[4].entry = (kprobe_opcode_t *) my_xen_l3_entry_update;
	//jphyper[5].entry = (kprobe_opcode_t *) my_dispatch_rw_block_io;

	//jphyper[0].kp.symbol_name="sys_ioctl";
	//jphyper[1].kp.symbol_name="xen_pgd_pin";
	jphyper[0].kp.symbol_name="xen_l2_entry_update";
	//jphyper[3].kp.symbol_name="__direct_remap_pfn_range";
	//jphyper[4].kp.symbol_name="xen_l3_entry_update";
	//jphyper[5].kp.symbol_name="dispatch_rw_block_io";
    printk("Fortune: Begin register.\n");
	for(i=0;i<jpnum;++i)
	{
		jphyper[i].kp.addr=0;
		if((ret=register_jprobe(&jphyper[i]))<0)
		{
			printk("Fortune: Error, register probe %d failed with return %d\n", i, ret);
			return -1;
		}
		if(jphyper[i].kp.addr == NULL)
		{
			printk("Fortune: Error, can't find %d address.\n", i);
			return -1;
		}
		printk("Fortune: Planted probe at %p, change %s\n", jphyper[i].kp.addr, jphyper[i].kp.symbol_name);
	}
	dir = proc_mkdir("jphyper", NULL);
	if(dir == NULL)
	{
		printk("Fortune: Can't create /proc/jphyper\n");
		return -1;
	}
	proc_aim = create_proc_entry("aim", PERMISSION, dir);
	if(proc_aim == NULL)
	{
		printk("Fortune: Can't create /proc/jphyper/aim\n");
		remove_proc_entry("jphyper", NULL);
		return -1;
	}
	proc_aim->read_proc = proc_read_aim;
	proc_aim->write_proc = proc_write_aim;
	proc_fault = create_proc_entry("fault", PERMISSION, dir);
	if(proc_fault == NULL)
	{
		printk("Fortune: Can't create /proc/jphyper/fault\n");
		remove_proc_entry("aim",dir);
		remove_proc_entry("jphyper", NULL);
		return -1;
	}
	proc_fault->read_proc = proc_read_fault;
	proc_fault->write_proc = proc_write_fault;
	proc_time = create_proc_entry("time", PERMISSION, dir);
	if(proc_time == NULL)
	{
		printk("Fortune: Can't create /proc/jphyper/time\n");
		remove_proc_entry("aim",dir);
		remove_proc_entry("fault",dir);
		remove_proc_entry("jphyper", NULL);
		return -1;
	}
	proc_time->read_proc = proc_read_time;
	proc_time->write_proc = proc_write_time;
	proc_id = create_proc_entry("id", PERMISSION, dir);
	if(proc_id == NULL)
	{
		printk("Fortune: Can't create /proc/jphyper/id\n");
		remove_proc_entry("aim", dir);
		remove_proc_entry("fault", dir);
		remove_proc_entry("time", dir);
		remove_proc_entry("jphyper", NULL);
		return -1;
	}
	proc_id->read_proc = proc_read_id;
	proc_id->write_proc = proc_write_id;
	proc_signal = create_proc_entry("signal", PERMISSION, dir);
	if(proc_signal == NULL)
	{
		printk("Fortune: Can't create /proc/jphyper/signal\n");
		remove_proc_entry("aim", dir);
		remove_proc_entry("fault", dir);
		remove_proc_entry("time", dir);
		remove_proc_entry("id", dir);
		remove_proc_entry("jphyper", NULL);
		return -1;
	}
	proc_signal->read_proc = proc_read_signal;
	proc_signal->write_proc = proc_write_signal;
	printk("Fortune: Create /proc/jphyper done.\n");
	return 0;
}

static void mvmhyper_exit(void)
{
	printk("mvmhyper exit.\n");
	for(i=0;i<jpnum;++i)
		unregister_jprobe(&jphyper[i]);
	remove_proc_entry("aim", dir);
	remove_proc_entry("fault", dir);
	remove_proc_entry("time", dir);
	remove_proc_entry("id", dir);
	remove_proc_entry("signal", dir);
	remove_proc_entry("jphyper", NULL);
}

module_init(mvmhyper_init);
module_exit(mvmhyper_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Little7");

