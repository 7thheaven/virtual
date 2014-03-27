/*
    A simple Linux kernel register fault injection  engine

    author :qinlei (octavian.qin@gmail.com)

    27/09/2009

    modified by Linye (linye@ftcl.hit.edu.cn)

    2010-08
*/

#include <linux/sys.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/user.h>
#include <linux/unistd.h>
//#include <linux/notifier.h>

#include <linux/version.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/sem.h>
#include <linux/list.h>
#include <asm/uaccess.h>  //copy_from_user
#include <linux/proc_fs.h>
#include <linux/limits.h>
#include <linux/kprobes.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/ptrace.h>
#include <linux/mman.h>

#include <linux/fs.h>

#include <linux/mm.h>      /*for pte manipulate*/
#include <linux/highmem.h>
#include <linux/sched.h>
#include <asm/segment.h>
#include <asm/pgalloc.h>
#include <asm/smp.h>
#include <asm/tlbflush.h>
//#include <asm-generic/sections.h>
#include <linux/wait.h>   /*for wait-queue*/
#include <linux/slab.h>   /*for kmalloc */
#include <linux/gfp.h>	   /*for flag parameter of kmalloc */
#include <asm/processor.h>  /*for task_pt_regs*/
#include <linux/kallsyms.h>  /*for kallsyms_lookup_name*/
//#include <asm/spinlock.h> /*for spinlock*/
#include <linux/random.h> /*for get_random_bytes*/

//#include <asm/pal.h>

#define __DEBUG__

#ifdef  __DEBUG__
#define DEBUGLOG(a)   printk  a
#else
#define DEBUGLOG(a) ""
#endif

#define STRINGLEN 17
#define PIDALL	-5

struct proc_dir_entry *reg_dir,*sig_number,*output,*pid,*DRControl,*faultlocation,*faulttype,*faultinterval;
static int interval = 0;

//control the record func action
static char f_inject;

//messages buffer 128*512 chars(8 pages in ia64).
#define PER_MSG_LEN 512
#define QUEUE_LEN 128
typedef struct element
{
    char msg[PER_MSG_LEN];
}element_t;

typedef struct queue
{
    element_t inj_log[QUEUE_LEN];
    int front;
    int rear;
}Q_t;

Q_t inj_info;

int addone(int i);

/*
char* Front(Q_t Q)
{
	if (IsEmpty(Q))
    return  NULL ;
  else
    return (Q.inj_log[Q.front].msg);
}

void  DeQueue ( Q_t* Q ) ;
{
	if (IsEmpty(Q))
    error  ("空队列") ;
  else
    Q->front = addone(Q->front);
}

void  EnQueue (char* x, Q_t* Q)
{
	if ( addone(addone(Q->rear))==Q->front )
    error ( "队列满") ;
  else {
    Q->rear = addone ( Q->rear ) ;
    Q->inj_log[ Q->rear ] = x ;
  }
}
 */
 
//main error inject function
void _inject_fault(struct pt_regs * regs, int location, int type, int pid, char* comm); //function declaration,the fault injection routine

unsigned long (*my_kallsyms_lookup_name)(const char *name);

//Queue Operations
int addone(int i)
{
    return ((i+1)%QUEUE_LEN);
}
/*
int IsEmpty(Q_t Q)
{
    if (addone(Q.rear) == Q.front)
        return  1;
    else
        return  0;
}*/

#define NOSIG 999
char sigstr[STRINGLEN];
int sig_n=NOSIG;
int proc_read_sig(char * page,char **start, off_t off, int count, int * eof,void * data)
{
    int len;
    len=sprintf(page,"%d",sig_n);
    return len;
}

char pidstr[STRINGLEN];
int pidval=-1;
int proc_read_pid(char * page,char **start, off_t off, int count, int * eof,void * data)
{
    int len;
    len=sprintf(page,"%d",pidval);
    return len;
}

int proc_write_pid(struct file *file,const char *buffer,unsigned long count,void * data)
{
    int len;
    int ret; 
    if (count <= STRINGLEN)
        len=STRINGLEN-1;
    else
    {
        printk("[ regFI(error) ]# too lage number for fault type!\n");
        return -1;
    }
    ret = copy_from_user(pidstr,buffer,len);
    pidstr[len]='\0';
    sscanf(pidstr,"%d",&pidval);
    DEBUGLOG(("[ regFI(msg) ]# target pid is %d\n",pidval));

    return len ;
}


char ftype[STRINGLEN];
int ftypeval=-1;
int proc_read_faulttype(char * page,char **start, off_t off, int count, int * eof,void * data)
{
    int len;
    len=sprintf(page,"%d",ftypeval);
    return len;
}

int proc_write_faulttype(struct file *file,const char *buffer,unsigned long count,void * data)
{
    int len;
    int ret;
    if (count <= STRINGLEN)
        len=STRINGLEN-1;
    else
    {
        printk("[ regFI(error) ]# too lage number for fault type!\n");
        return -1;
    }
    ret = copy_from_user(ftype,buffer,len);
    ftype[len]='\0';
    sscanf(ftype,"%d",&ftypeval);
    DEBUGLOG(("[ regFI(msg) ]# fault-type is %d\n",ftypeval));

    return len ;
}

char flocation[STRINGLEN];
int flocationval=-1;
int proc_read_faultlocation(char * page,char **start, off_t off, int count, int * eof,void * data)
{
    int len;
    len=sprintf(page,"%d",flocationval);
    return len;
}

int proc_write_faultlocation(struct file *file,const char *buffer,unsigned long count,void * data)
{
    int len;
    int ret;
    if (count <= STRINGLEN)
        len=STRINGLEN-1;
    else
    {
        printk("[ regFI(error) ]# too lage number for fault location!\n");
        return -1;
    }
    ret = copy_from_user(flocation,buffer,len);
    flocation[len]='\0';
    sscanf(flocation,"%d",&flocationval);
    DEBUGLOG(("[ regFI(msg) ]# fault-location is %d\n",flocationval));

    return len ;
}


char finterval[STRINGLEN];
int finterval2=-1;
int proc_read_faultinterval(char * page,char **start, off_t off, int count, int * eof,void * data)
{
    int len;
    len=sprintf(page,"%d",finterval2);
    return len;
}

int proc_write_faultinterval(struct file *file,const char *buffer,unsigned long count,void * data)
{
    int len;
    int ret;
    if (count <= STRINGLEN)
        len=STRINGLEN-1;
    else
    {
        printk("[ regFI(error) ]# too lage number for fault interval!\n");
        return -1;
    }
    ret = copy_from_user(finterval,buffer,len);
    finterval[len]='\0';
    sscanf(finterval,"%d",&finterval2);
    DEBUGLOG(("[ regFI(msg) ]# fault-interval is %d\n",finterval2));

    return len ;
}


#define PAGESIZE 512
char global_message[PAGESIZE]={'\0','\0'};
static struct semaphore sem;

int proc_read_output(char * page,char **start, off_t off, int count, int* eof,void * data)
{
    int len=0;

    if (down_interruptible(&sem))
    	return -ERESTARTSYS;

  	if (addone(inj_info.rear) == inj_info.front)
    {
        len = sprintf(page,"#");
				up(&sem);
        return len;
   	}
    else
    {
    	page[0]='\0';
    	
    	do
    	{
        len += sprintf(page+strlen(page), "%s", inj_info.inj_log[inj_info.front].msg);
        //len = strlen(page);
        inj_info.front = addone(inj_info.front);
      }while(addone(inj_info.rear) != inj_info.front);
      
      up(&sem);
      return len;
    }
	
}

#define BUFLEN 1024
char global_buffer[BUFLEN];

int proc_read_DRControl(char * page,char **start, off_t off, int count, int * eof,void * data)
{
    int len;

    global_buffer[BUFLEN-1] = '\0';
    len=sprintf(page,"%s",global_buffer);

    return len;
}

int proc_write_DRControl(struct file *file,const char *buffer,unsigned long count,void * data)
{
    int len;
    int ret; 
    if (count >= BUFLEN)
        len=BUFLEN-1;
    else
        len=count;
    ret = copy_from_user(global_buffer,buffer,len);
    global_buffer[len]='\0';

    if (global_buffer[0]=='s'&&global_buffer[1]=='t'&&global_buffer[2]=='a'&&global_buffer[3]=='r'&&global_buffer[4]=='t')
    {
        inj_info.front = 0 ;
        inj_info.rear = QUEUE_LEN - 1;
        f_inject = 'Y';
    }
    else if (global_buffer[0]=='s'&&global_buffer[1]=='t'&&global_buffer[2]=='o'&&global_buffer[3]=='p')
    {
        f_inject = 'N';
        
        if (down_interruptible(&sem))
    	      return -ERESTARTSYS;
    	      
        if ( addone(addone(inj_info.rear))==inj_info.front )
        {
            /*error:队列满*/
            sprintf(inj_info.inj_log[inj_info.rear].msg,"stop");
        }
        else
        {
            inj_info.rear = addone(inj_info.rear);
            sprintf(inj_info.inj_log[inj_info.rear].msg,"stop");
            //inj_info->inj_log[inj_info->rear ] = x ;
        }
        up(&sem);

	printk("[ regFI(msg) ]# STOP\n");
    }
    else
        printk("[ regFI(error) ]# invalid command! support: start, stop, init\n");

    return len ;

}

/*******************************************************************
          					fault models live here
********************************************************************/

static int myRand(void)
{
    static int randomval=0;

    get_random_bytes(&randomval,sizeof(int));

    return randomval;
}

/*
low 5 bits
return (0,31)
*/
int randBit(void)
{
    return (myRand() & 0x1f);
}

/*
high 56 bits
地址均为16的倍数
*/
int randAddr(void)
{
    return (myRand() & 0xfffffffffffffff0L);
}

/**********************************************************
	location of the fault:        				 //memory fault
					  														 //cpu register fault
***********************************************************/
void _inject_fault(struct pt_regs *regs, int location, int type, int pid, char* comm)
{
    long data = 0;
    long data0 = 0;
    long *addr = (long*)regs;

    int i;
    char *message;

    if (f_inject == 'N')
        return;
    
    if (down_interruptible(&sem))
     	  return;
        
    if ( addone(addone(inj_info.rear))==inj_info.front )
    {
        /*error:队列满*/
        sprintf(inj_info.inj_log[inj_info.rear].msg,"caution : buf is full, messages have been dropped\n");
    }
    else
    {
        inj_info.rear = addone(inj_info.rear);
        sprintf(inj_info.inj_log[inj_info.rear].msg,"injecting fault to pid:%d,process name:%s,fault-type:%d,location:%d,interval:%d\n", pid,comm,ftypeval,flocationval,finterval2);
        //inj_info->inj_log[inj_info->rear ] = x ;
    }
    up(&sem);

    printk("[ regFI(msg) ]# injecting fault to pid:%d process name:%s fault-type:%d, location:%d, interval:%d\n",pid,comm,type,location,finterval2);
    
    if (location >= 0 && location <= sizeof(struct pt_regs)/sizeof(long)) //register in struct pt_regs fault in ia32 linux 2.6.28
        data = *(addr + location);
    else
    {
        if (down_interruptible(&sem))
    	      return;
    	  
        if ( addone(addone(inj_info.rear))==inj_info.front )
        {
            /*error:队列满*/
            sprintf(inj_info.inj_log[inj_info.rear].msg,"caution : buf is full, messages have been dropped\n");
        }
        else
        {
            inj_info.rear = addone(inj_info.rear);
            sprintf(inj_info.inj_log[inj_info.rear].msg,"invalid location of fault, register number 0-%lu!\n",sizeof(struct pt_regs)/sizeof(long));
            //inj_info->inj_log[inj_info->rear ] = x ;
        }
        up(&sem);

        printk("[ regFI(error) ]# invalid location of fault, register number 0-%lu!\n",sizeof(struct pt_regs)/sizeof(long));
        return;
    }

    data0 = data;
    message=kmalloc(PAGESIZE,GFP_KERNEL);
    for (i=0;i<PAGESIZE;i++)
        message[i]='\0';

    switch (type)
    {
    case 0:									//
        data ^= (0x00000001L << randBit());

        sprintf(message+strlen(message),"_inject_1_bit_flip,%lx->%lx\n",data0,data);

        printk("[ regFI(msg) ]# _inject_1_bit_flip,%lx -> %lx\n",data0,data);
        break;
    case 1:									//
        data ^= (0x00000001L << randBit());
        data ^= (0x00000001L << randBit());

        sprintf(message+strlen(message),"_inject_2_bit_flip,%lx -> %lx\n",data0,data);

        printk("[ regFI(msg) ]# _inject_2_bit_flip,%lx -> %lx\n",data0,data);
        break;
    case 2:									//
        data &= (~(0x00000001L << randBit()));

        sprintf(message+strlen(message),"_inject_1_bit_0,%lx -> %lx\n",data0,data);

        printk("[ regFI(msg) ]# _inject_1_bit_0,%lx -> %lx\n",data0,data);
        break;
    case 3:									//
        data &= (~(0x00000001L << randBit()));
        data &= (~(0x00000001L << randBit()));

        sprintf(message+strlen(message),"_inject_2_bit_0,%lx -> %lx\n",data0,data);

        printk("[ regFI(msg) ]# _inject_2_bit_0,%lx -> %lx\n",data0,data);
        break;
    case 4:									//
        data |= (0x00000001L << randBit());

        sprintf(message+strlen(message),"_inject_1_bit_1,%lx -> %lx\n",data0,data);

        printk("[ regFI(msg) ]# _inject_1_bit_1,%lx -> %lx\n",data0,data);

        break;
    case 5:									//
        data |= (0x00000001L << randBit());
        data |= (0x00000001L << randBit());

        sprintf(message+strlen(message),"_inject_2_bit_1,%lx -> %lx\n",data0,data);

        printk("[ regFI(msg) ]# _inject_2_bit_1,%lx -> %lx\n",data0,data);
        break;
    case 6:									//
        data ^= myRand();

        sprintf(message+strlen(message),"_inject_word_error,%lx -> %lx\n",data0,data);

        printk("[ regFI(msg) ]# _inject_word_error,%lx -> %lx\n",data0,data);
        break;
    case 7:									//
        data &= 0xffffffffffffff00L;

        sprintf(message+strlen(message),"_inject_8_low_0,%lx -> %lx\n",data0,data);

        printk("[ regFI(msg) ]# _inject_8_low_0,%lx -> %lx\n",data0,data);
        break;
    case 8:									//
        data |= 0x000000ffL;

        sprintf(message+strlen(message),"_inject_8_low_1,%lx -> %lx\n",data0,data);

        printk("[ regFI(msg) ]# _inject_8_low_1,%lx -> %lx\n",data0,data);
        break;
    case 9:									//
        data ^= (myRand() & 0x000000ffL);
        sprintf(message+strlen(message),"_inject_8_low_error,%lx -> %lx\n",data0,data);
        printk("[ regFI(msg) ]# _inject_8_low_error,%lx -> %lx\n",data0,data);
        break;
    case 10:									//
        data += 1;
        sprintf(message+strlen(message),"_inject_plus_1,%lx -> %lx\n",data0,data);
        printk("[ regFI(msg) ]# _inject_plus_1,%lx -> %lx\n",data0,data);
        break;
    case 11:									//
        data += 2;
        sprintf(message+strlen(message),"_inject_plus_2,%lx -> %lx\n",data0,data);
        printk("[ regFI(msg) ]# _inject_plus_2,%lx -> %lx\n",data0,data);
        break;
    case 12:									//
        data += 3;
        sprintf(message+strlen(message),"_inject_plus_3,%lx -> %lx\n",data0,data);
        printk("[ regFI(msg) ]# _inject_plus_3,%lx -> %lx\n",data0,data);
        break;
    case 13:									//
        data += 4;
        sprintf(message+strlen(message),"_inject_plus_4,%lx -> %lx\n",data0,data);
        printk("[ regFI(msg) ]# _inject_plus_4,%lx -> %lx\n",data0,data);
        break;
    case 14:									//
        data += 5;
        sprintf(message+strlen(message),"_inject_plus_5,%lx -> %lx\n",data0,data);
        printk("[ regFI(msg) ]# _inject_plus_5,%lx -> %lx\n",data0,data);
        break;
    default:

        sprintf(message+strlen(message),"unknown fault type,only 1-15 supported!");

        printk("[ regFI(error) ]# unknown fault type,only 1-15 supported!\n");
        break;
    }

    if (location >= 0 && location <= sizeof(struct pt_regs)/sizeof(long)) //set back pt_regs with faulty value
        *(addr + location) = data ;


//notify output message of this injection
    message[strlen(message)-1]='\n';
    message[strlen(message)]='\0';
    message[strlen(message)+1]='\0';
    printk("\n");

    if (f_inject == 'N')
    {
        kfree(message);
        return;
    }

    if (down_interruptible(&sem))
    {
    	  kfree(message);
    	  return;
    }
    if ( addone(addone(inj_info.rear))==inj_info.front )
    {
        /*error:队列满*/
        sprintf(inj_info.inj_log[inj_info.rear].msg,"caution : buf is full, messages have been dropped.\n");
    }
    else
    {
        inj_info.rear = addone(inj_info.rear);
        strcpy(inj_info.inj_log[inj_info.rear].msg,message);
        //inj_info->inj_log[inj_info->rear ] = x ;
    }
    up(&sem);

    kfree(message);
    
    return;
}

/*
void show_pt_regs(struct pt_regs *regs)
{
    unsigned long *p;
    int i;
    p=(unsigned long *)regs;
    for (i=0;i< sizeof(struct pt_regs)/sizeof(unsigned long);i++)
    {
        printk("%d:%ld	 ",i,*p);
        if (i !=0 && i%6 == 0)
            printk("\n");
        p++;
    }
}*/



static int jforce_sig_info(int sig,struct siginfo *info,struct task_struct *t)
{
    printk("[ regFI(warning) ]# kernel is sending signal %d to process pid: %d, comm: %s\n",sig,t->pid,t->comm);
    sig_n = sig;

    if (f_inject == 'N')
    {
        jprobe_return();
        return 0;
    }

    if (down_interruptible(&sem))
    {
    	  jprobe_return();
    	  return -ERESTARTSYS;
    }
    
    if ( addone(addone(inj_info.rear))==inj_info.front )
    {
        /*error:队列满*/
        sprintf(inj_info.inj_log[inj_info.rear].msg,"caution : buf is full, messages have been dropped\n");
    }
    else
    {
        inj_info.rear = addone(inj_info.rear);
        sprintf(inj_info.inj_log[inj_info.rear].msg,"warning : kernel is sending signal %d to process pid: %d, comm: %s\n",sig,current->pid,current->comm);
    }
    up(&sem);
    
    jprobe_return();
    return 0;
}


struct jprobe jprobe1 =
{
    .entry				           = jforce_sig_info,
    .kp = {
        .symbol_name = "force_sig_info",
    },
};


//kprobe for do_timer() function
struct kprobe kp4 =
{
    .symbol_name	= "do_timer",
};


/**************************************************************************************
*** inject register fault at the beginning of do_timer() function
***************************************************************************************/

static int handler_pre4(struct kprobe *p, struct pt_regs *regs)
{
    //printk("Enter handler_pre4\n");
    if (f_inject == 'Y')//enable cpu register fault inject
    {
        //to do

        struct task_struct *task,*p;
        struct pt_regs *kstack_regs;
        struct list_head *pos;
        task=&init_task;
        
        if (finterval2 > 0)
        {
            interval++;

            if (interval >= finterval2)
            {
                list_for_each(pos,&task->tasks)
                {
                    p=list_entry(pos, struct task_struct, tasks);
                    
                    if (pidval == p->pid || pidval == PIDALL)
                    {
                        //inject faults to registers here
                        //printk("inject register fault in os kernel to pid: %d\n", p->pid);
                        kstack_regs = task_pt_regs(p);
                        sig_n = NOSIG;
                        _inject_fault( kstack_regs, flocationval, ftypeval,p->pid,p->comm);
                        interval = 0;
                        break;
                    }
                }
                finterval2=0;
            }
        }
    }

    return 0;
}

static void __exit exit_DR(void)
{
    DEBUGLOG(("******* UNLOADING IA64 REG-FAULT INJECT ENGINE *******\n"));

    //remove all the files in /proc
    remove_proc_entry("pid",reg_dir);
    remove_proc_entry("DRControl",reg_dir);
    remove_proc_entry("output",reg_dir);
    remove_proc_entry("faultlocation",reg_dir);
    remove_proc_entry("faultinterval",reg_dir);
    remove_proc_entry("faulttype",reg_dir);
    remove_proc_entry("signal",reg_dir);
    remove_proc_entry("reg_inject",NULL);
    
    //unregister all the kernel function probes
    unregister_jprobe(&jprobe1);
    unregister_kprobe(&kp4);                          //do_timer probe
    printk("[ regFI(msg) ]# jprobe at %p unregistered.\n",	jprobe1.kp.addr);
    printk("[ regFI(msg) ]# kprobe at %p unregistered.\n",	kp4.addr);

    return;
}


static int __init init_DR(void)
{
    int ret;

    DEBUGLOG(("******* LOADING REG-FAULT INJECT ENGINE *******\n"));
    reg_dir = proc_mkdir("reg_inject",NULL);

    pid = create_proc_entry("pid",0644,reg_dir);
    pid->read_proc = proc_read_pid;
    pid->write_proc = proc_write_pid;
//    pid->owner=THIS_MODULE;

    sig_number = create_proc_entry("signal",0644,reg_dir);
    sig_number->read_proc = proc_read_sig;
//    sig_number->owner=THIS_MODULE;
    
    output = create_proc_entry("output",0644,reg_dir);
    output->read_proc = proc_read_output;
//    output->owner=THIS_MODULE;

    faultlocation = create_proc_entry("faultlocation",0644,reg_dir);
    faultlocation->read_proc = proc_read_faultlocation;
    faultlocation->write_proc = proc_write_faultlocation;
//    faultlocation->owner=THIS_MODULE;

    faulttype = create_proc_entry("faulttype",0644,reg_dir);
    faulttype->read_proc = proc_read_faulttype;
    faulttype->write_proc = proc_write_faulttype;
//    faulttype->owner=THIS_MODULE;

    faultinterval = create_proc_entry("faultinterval",0644,reg_dir);
    faultinterval->read_proc = proc_read_faultinterval;
    faultinterval->write_proc = proc_write_faultinterval;
//    faultinterval->owner=THIS_MODULE;

    DRControl = create_proc_entry("DRControl",0644,reg_dir);
    DRControl->read_proc = proc_read_DRControl;
    DRControl->write_proc = proc_write_DRControl;
//    DRControl->owner=THIS_MODULE;

    //init semphore and wait queue structures
    init_MUTEX(&sem);

    ret = register_jprobe(&jprobe1);
    if (ret < 0)
    {
        printk("[ regFI(error) ]# register_jprobe jprobe1 failed, returned %d\n", ret);
        return ret;
    }
    printk("[ regFI(msg) ]# Planted jprobe at force_sig_info: %p\n", jprobe1.kp.addr);

    kp4.pre_handler = handler_pre4;
    ret = register_kprobe(&kp4);
    if (ret < 0)
    {
        printk("[ regFI(error) ]# register_kprobe kp4 failed, returned %d\n", ret);
        return ret;
    }
    printk("[ regFI(msg) ]# Planted kprobe at do_timer: %p\n", kp4.addr);
    interval = 0;

    //init /proc/output bufferList
    //Head = Tail = NULL;
    inj_info.front = 0;
    inj_info.rear = QUEUE_LEN - 1;
    f_inject = 'N';
    
    printk("[ regFI(msg) ]# Queue front : %d\n",inj_info.front);
    printk("[ regFI(msg) ]# Queue rear  : %d\n",inj_info.rear);
    printk("[ regFI(msg) ]# inject flag : %c\n",f_inject);

    return 0; //-EINVAL;
}

/*
    main module init/exit
*/

module_init(init_DR);
module_exit(exit_DR);

/* taint-safe */
MODULE_LICENSE("GPL");
