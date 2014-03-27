/*
*  Author: HIT CS HDMC team.
*  Create: 2010-3-12 8:50
*  Last modified: 2010-6-13 14:06:20
*  Description:
*  	Memory fault injection engine running as a kernel module.
*		This module will create "/proc/memoryEngine/" directory and 9 proc nodes.
*   Write paramenters and request to these proc nodes and read the output from related proc node.
*/

#include "memoryEngine.h"

/*
*	proc entries
*/
struct proc_dir_entry *dir = NULL;
struct proc_dir_entry *proc_pid = NULL;				/// write only
struct proc_dir_entry *proc_va = NULL;				/// write only
struct proc_dir_entry *proc_ctl = NULL;				/// write only
struct proc_dir_entry *proc_kFuncName = NULL;	/// write only
struct proc_dir_entry *proc_val = NULL;				/// rw
struct proc_dir_entry *proc_signal = NULL;		/// rw
struct proc_dir_entry *proc_pa = NULL;				/// read only
struct proc_dir_entry *proc_taskINfo = NULL;	/// read only

/*
* proc node values
*/
int pid;									/// pid
unsigned long va;					/// virtualAddr
unsigned long pa;					/// physicalAddr
int ctl;									/// ctl
int signal;								/// signal
char kFuncName[MAX_LINE]; /// kFuncName
long memVal;							/// memVal

unsigned long ack_pa;			/// physicalAddr
unsigned long ack_va;			/// virtualAddr
int ack_signal;						/// signal
int ret;
char taskInfo[PAGE_SIZE];	/// taskInfo
///////////////////////////////////////////////
unsigned long userspace_phy_mem;
long orig_pa_data;
long new_pa_data;

int faultInterval;

/*
*	kprobe
*/
static struct kprobe kp_kFunc;
/*
struct jprobe jprobe1 =
{
    .entry				           = jforce_sig_info,
    .kp = {
        .symbol_name = "force_sig_info",
    },
};
*/

//ʱ���ж�������
static int count = 0;
//����ԭʼ����
static long orig_code = 0;

/*
*  process the request
*/
void do_request(void)
{
	struct task_struct *task = NULL;
	unsigned long pa = 0;
	long kernel_va = 0;
	int status;
	
	/// get a task's memory map information
	if(ctl == REQUEST_TASK_INFO)
	{
		dbginfo("Rcv request:Get task info\n");
		memset(taskInfo,'\0',sizeof(taskInfo));
		if(pid <= 0)
		{
			ack_signal = ACK_TASK_INFO;
			return;
		}
		task = findTaskByPid(pid);
		if( task != NULL )
		{
			dbginfo("findTaskByPid is OK\n");
			getTaskInfo(task, taskInfo, sizeof(taskInfo));
		}
		ack_signal = ACK_TASK_INFO;
		
		return;
	}
	/// convert a process's linear address to physical address
	else if(ctl == REQUEST_V2P)
	{
		task = findTaskByPid(pid);
		if( task == NULL )
		{
			dbginfo("No such process\n");
			ack_pa = -1;
			ack_signal = ACK_V2P;
			return;
		}
		if( task->mm == NULL )
		{
			ack_pa = -1;
			ack_signal = ACK_V2P;
			return;
		}
		ack_pa = v2p(task->mm,va,&status);
		if(ack_pa == FAIL)
		{
			dbginfo("No physical address\n");
		}
		ack_signal = ACK_V2P;
		return;
	}
	/// convert kernel virtual address to physical address
	else if(ctl == REQUEST_KV2P)
	{
		ack_pa = kv2p(va,&status);
		if(pa == FAIL)
		{
			dbginfo("No physical address\n");
		}
		ack_signal = ACK_KV2P;
		return;
	}
	/// get kernel function's addr(kernel virtual address)
	else if(ctl == REQUEST_KFUNC_VA)
	{
		ack_va = kFunc2v(kFuncName);
		ack_signal = ACK_KFUNC_VA;
		return;
	}
	/// �����ȡ�ں˺�����ʼ��ַ����
	else if(ctl == REQUEST_READ_KFUNC)
	{
		kernel_va = kFunc2v(kFuncName);
		memVal = *((long *)kernel_va);
		ack_signal = ACK_READ_KFUNC;
	}
	/// �����д�ں˺�����ʼ��ַ����
	else if(ctl == REQUEST_WRITE_KFUNC)
	{
		//����kprobe���ڵ�һ�ε���do_timer()ʱ��ע�����
		int ret;
		count = 0;
		if(strlen(kFuncName) > 0)
		{
			faultInterval = 1;	//���Ͻ�����һ��ʱ������
			kp_kFunc.addr = 0;
			kp_kFunc.symbol_name = kFuncName;
			kp_kFunc.pre_handler = handler_pre_kFunc;
			ret = register_kprobe(&kp_kFunc);
			if(ret < 0)
			{
				dbginfo("Fained to register kprobe\n");
				ack_signal = ACK_WRITE_KFUNC;
				return;
			}
			
			//�ȴ�����ע�����
			dbginfo("start count\n");
			int temp=0;
			while(1)
			{
				if(count == -1)
				{
					unregister_kprobe(&kp_kFunc);
					dbginfo("recovery\n");
					break;
				}
				if(temp == -1)
				{
					break;
				}
				temp++;
				//dbginfo("count:%d\n",count);
			}
		}
		ack_signal = ACK_WRITE_KFUNC;
		dbginfo("Success to inject MTTR fault\n");
		return;
	}
	
}

/*
*  get a task's memory map information
*/
int getTaskInfo(struct task_struct *pTask, char *pData, int length)
{
	struct mm_struct *pMM;
	struct vm_area_struct *pVMA;
	struct vm_area_struct *p;
	char file[MAX_LINE];
	struct dentry *pPath = NULL;
	char *end, *start;
	char *info = pData;
	
	unsigned long phy_addr;
	unsigned long start_va,end_va;
	int status;
	
	if(pTask == NULL) { return FAIL; }
	dbginfo("pid is %d\n", pTask->pid);
	dbginfo("process state is %d\n", pTask->state);
	if((pMM = pTask->mm) == NULL)
	{
		dbginfo("pTask->mm is null\n");
		if((pMM = pTask->active_mm) == NULL)
		{
			dbginfo("pTask->active_mm is null\n");
			return FAIL;
		}
	}
	
	memset(pData, '\0', length);
	//ǰ19���ֶ��ǹ��ڽ����ڴ���Ϣ��������Ϣ
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->total_vm, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->locked_vm, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->shared_vm, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->exec_vm, DELIMITER);

	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->stack_vm, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->reserved_vm, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->def_flags, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->nr_ptes, DELIMITER);
	
	dbginfo("pMM->total_vm : %lx|\n", pMM->start_code);

	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->start_code, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->end_code, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->start_data, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->end_data, DELIMITER);
	
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->start_brk, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->brk, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->start_stack, DELIMITER);
	
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->arg_start, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->arg_end, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->env_start, DELIMITER);
	safe_sprintf(pData, length, info+strlen(info), "%lx%c", pMM->env_end, DELIMITER);
	
	pVMA = pMM->mmap;
	if(pVMA == NULL) { return OK; }
	for(p=pVMA; p!=NULL; p=p->vm_next)
	{
		//��ʼ��ַ
		safe_sprintf(pData, length, info+strlen(info), "%lx %lx ", p->vm_start, p->vm_end);
		//����
		if(p->vm_flags & VM_READ)
		{	safe_sprintf(pData, length, info+strlen(info), "r"); }
		else
		{	safe_sprintf(pData, length, info+strlen(info), "-"); }
			
		if(p->vm_flags & VM_WRITE)
		{	safe_sprintf(pData, length, info+strlen(info), "w"); }
		else
		{	safe_sprintf(pData, length, info+strlen(info), "-"); }
			
		if(p->vm_flags & VM_EXEC)
		{	safe_sprintf(pData, length, info+strlen(info), "x"); }
		else
		{	safe_sprintf(pData, length, info+strlen(info), "-"); }
			
		if(p->vm_flags & VM_SHARED)
		{	safe_sprintf(pData, length, info+strlen(info), "s"); }
		else
		{	safe_sprintf(pData, length, info+strlen(info), "p"); }
			
		//��Ӧ�ļ���
		if(p->vm_file != NULL)
		{
			if(p->vm_file->f_dentry != NULL)
			{
				safe_sprintf(pData, length, info+strlen(info), " ");
				memset(file,'\0',sizeof(file));
				for(pPath = p->vm_file->f_dentry; pPath != NULL; pPath = pPath->d_parent)
				{
					if(strcmp(pPath->d_name.name,"/") != 0)
					{
						strcpy(file + strlen(file), pPath->d_name.name);
						strcpy(file + strlen(file), "/");
						continue;
					}
					break;
				}
				do
				{
					end = file + strlen(file) - 1;
					for(start = end - 1; *start != '/' && start > file; start--);
					if(*start == '/')	{start++;}
					*end = '\0';
					
					safe_sprintf(pData, length, info+strlen(info), "/%s", start);
					*start = '\0';
				} while(start > file);
			}
		}
		safe_sprintf(pData, length, info+strlen(info), "%c", DELIMITER);
		
		//��Ӧ�����ַҳ
		start_va = p->vm_start;
		end_va = p->vm_end;
		while(end_va > start_va)
		{
			safe_sprintf(pData, length, info+strlen(info), "%lx-%lx\t", start_va, start_va + PAGE_SIZE);
			phy_addr = v2p(pMM, start_va, &status);
			if(phy_addr != FAIL)
			{	
				safe_sprintf(pData, length, info+strlen(info), "va:0x%lx <--> pa:0x%lx", start_va, phy_addr);
			}
			start_va += PAGE_SIZE;
			safe_sprintf(pData, length, info+strlen(info), "%c", DELIMITER);
		}
		
		safe_sprintf(pData, length, info+strlen(info), "%c", DELIMITER);
	}
	return OK;
}

/*
*  
*/
static int handler_pre_kFunc(struct kprobe *p, struct pt_regs *regs)
{
	unsigned long va;
	va = (unsigned long)p->addr;
	if(va <= 0) { return OK; }
		
	//��һ�δ���
	if(count == 0)
	{
		//��ȡǰ64�ֽڣ�ע�����
		orig_code = *((long *)va);
		//_inject_fault(va,memVal);
		*((long *)va) = memVal;	//����
		return OK;
	}
	count ++;
	//�������ʱ��
	if(count == faultInterval + 1)
	{
		//�ָ�code
		*((long *)va) = orig_code;
		count = -1;
	}
	return OK;
}

/*
*	find_task_by_pid maybe not supported
*	O(n) is fine :)
*/
struct task_struct * findTaskByPid(pid_t pid)
{
	struct task_struct *task = NULL;
	for_each_process(task)
	{
		if(task->pid == pid)
			return task;
	}
	return NULL;
}
pte_t * getPte(struct mm_struct *pMM,unsigned long va)
{
	pgd_t *pgd = NULL;
	pmd_t *pmd = NULL;
	pud_t *pud = NULL;
	pte_t *pte = NULL;
	
	///get the pdg entry pointer
	pgd =  pgd_offset(pMM, va);
	if(pgd_none(*pgd)) { return NULL; }

	pud = pud_offset(pgd,va); 
	if(pud_none(*pud)) { return NULL; }
		
	pmd = pmd_offset(pud,va);
	if(pmd_none(*pmd)) { return NULL; }
	
	pte = pte_offset_kernel(pmd,va);
	if(pte_none(*pte)) { return NULL; }
	if(!pte_present(*pte)) { return NULL; }
	return pte;
}

/*
* convert a process's linear address to physical address
*/
unsigned long v2p(struct mm_struct *pMM,unsigned long va,int *pStatus)
{
	pgd_t *pgd = pgd_offset(pMM, va);
	if(pgd_present(*pgd))
	{
		pud_t *pud = pud_offset(pgd, va);
		if(pud_present(*pud))
		{
			pmd_t *pmd = pmd_offset(pud, va);
			if(pmd_present(*pmd))
			{
				pte_t *pte = pte_offset_kernel(pmd, va);
				if(pte_present(*pte))
				{
					//*(unsigned long *)pte | 0x;
					pte_mkclean(*pte);
					dbginfo("In kernel pte:0x%lx\n",*(unsigned long*)pte);
					//return ((unsigned long)page_address(pte_page(*pte)) | (va & ~PAGE_MASK));
					return (( pte_val(*pte) & PAGE_MASK ) | (va & ~PAGE_MASK));
				}
			}
		}
	}
	else
	{
		return FAIL;
	}
/*
	pte_t *pte = NULL;
	unsigned long phyaddress = FAIL;
	
	pte = getPte(pMM, va);
	if(pte != NULL)
	{
		phyaddress = (pte_val(*pte) & PAGE_MASK) | (va & ~PAGE_MASK);
	}
	return phyaddress;
*/
}

/*
*  convert kernel virtual address to physical address
*/
long kv2p(unsigned long va,int *pStatus)
{
	if(va < 0)
		return FAIL;
	if(__pa(va) >= 0)
		return __pa(va);
	return FAIL;
}

/*
*  get kernel function's addr(kernel virtual address)
*	 the kernel function should be looked up in the System.map
*/
static struct kprobe kp;
long kFunc2v(char *funcName)
{
	int ret;
	unsigned long va;

	kp.addr = 0;
	kp.symbol_name = funcName;
	ret = register_kprobe(&kp);
	if(ret < 0)
	{
		dbginfo("Fained to register kprobe\n");
		return FAIL;
	}
	va = (unsigned long)kp.addr;
	unregister_kprobe(&kp);
	if(va == 0)
		return FAIL;
	return va;
}



/*
*  
*/
struct vm_area_struct * getVMA(struct mm_struct *pMM,unsigned long va)
{
	struct vm_area_struct *p;
	if(pMM == NULL) return NULL;
	p = pMM->mmap;
	if(p == NULL) return NULL;
	
	for(; p != NULL; p = p->vm_next)
	{
		if( va >= p->vm_start && va < p->vm_end )
		{
			return p;
		}
	}
	return NULL;
}


/*
*  
*/
int setVMAFlags(struct mm_struct *pMM,unsigned long va,int *pStatus,int flags)
{
	struct vm_area_struct *p;
	p = getVMA(pMM,va);
	if(p == NULL) return FAIL;
	
	if(flags > 0)
	{
		p->vm_flags |= VM_WRITE;
		p->vm_flags |= VM_SHARED;
	}
	if(flags == 0)
	{
		p->vm_flags &= ~VM_WRITE;
		p->vm_flags &= ~VM_SHARED;
	}
	else { return FAIL; }
	return OK;
}

/*
*  
*/
int setPageFlags(struct mm_struct *pMM,unsigned long va,int *pStatus,int flags)
{
	pte_t *pte = NULL;
	pte_t ret;
	pte = getPte(pMM, va);
	if( pte == NULL ) { return FAIL; }
	if(flags > 0)
	{
		ret = pte_mkwrite(*pte);
	}
	else if(flags == 0)
	{
		ret = pte_wrprotect(*pte);
	}
	else { return FAIL;	}
	return OK;
}

/*
*  
*/
int proc_write_pid(struct file *file,const char __user *buffer,unsigned long count,void * data)
{
	int iRet;
	char sPid[MAX_LINE];
	
	if(count <= 0) { return FAIL; }
	memset(sPid, '\0', sizeof(sPid));
	iRet = copy_from_user(sPid,buffer,count);
	if(iRet) { return FAIL; }
	iRet = sscanf(sPid,"%d",&pid);
	if(iRet != 1) { return FAIL; }
	dbginfo("Rcv pid:%d\n",pid);
  return count;
}

/*
*  
*/
int proc_read_virtualAddr(char * page,char **start, off_t off, int count, int * eof,void * data)
{
	int iLen;
	iLen = sprintf(page, "%lx", ack_va);
	return iLen;
}

/*
*  
*/
int proc_write_virtualAddr(struct file *file,const char *buffer,unsigned long count,void * data)
{
	int iRet;
	char sVa[MAX_LINE];
	
	if(count <= 0) { return FAIL; }
	memset(sVa, '\0', sizeof(sVa));
	iRet = copy_from_user(sVa, buffer, count);
	if(iRet) { return FAIL; }
	iRet = sscanf(sVa,"%lx",&va);
	if(iRet != 1) { return FAIL; }
	dbginfo("Rcv virtual addr:0x%lx\n",va);
	return count;
}

/*
*  
*/
int proc_write_ctl(struct file *file,const char *buffer,unsigned long count,void * data)
{
	dbginfo("in proc_write_ctl()\n");
	int iRet;
	char sCtl[MAX_LINE];
	
	if(count <= 0) { return FAIL; }
	memset(sCtl, '\0', sizeof(sCtl));
	iRet = copy_from_user(sCtl, buffer, count);
	if(iRet) { return FAIL; }
	iRet = sscanf(sCtl,"%d",&ctl);
	if(iRet != 1) { return FAIL; }
	dbginfo("befor de_request()\n");
	do_request();
	dbginfo("after de_request()\n");
	return count;
}

/*
*  
*/
int proc_read_signal(char * page,char **start, off_t off, int count, int * eof,void * data)
{
	int iLen;
	iLen = sprintf(page, "%d", ack_signal);
	return iLen;
}

/*
*  
*/
int proc_write_signal(struct file *file,const char *buffer,unsigned long count,void * data)
{
	int iRet;
	char sSignal[MAX_LINE];
	
	if(count <= 0) { return FAIL; }
	memset(sSignal, '\0', sizeof(sSignal));
	iRet = copy_from_user(sSignal, buffer, count);
	if(iRet) { return FAIL; }
	iRet = sscanf(sSignal,"%d",&signal);
	if(iRet != 1) { return FAIL; }
	dbginfo("Rcv signal:%d\n",signal);
	return count;
}

/*
*  
*/
int proc_read_pa(char * page,char **start, off_t off, int count, int * eof,void * data)
{
	int iLen;
	iLen = sprintf(page, "%lx", ack_pa);
	return iLen;
}

/*
*  
*/
int proc_write_pa(struct file *file,const char *buffer,unsigned long count,void * data)
{
	int iRet;
	char sPa[MAX_LINE];
	
	if(count <= 0) { return FAIL; }
	memset(sPa, '\0', sizeof(sPa));
	iRet = copy_from_user(sPa, buffer, count);
	if(iRet) { return FAIL; }
	iRet = sscanf(sPa,"%lx",&pa);
	if(iRet != 1) { return FAIL; }
	dbginfo("Rcv pa:0x%lx\n",pa);
	return count;
}

/*
*  
*/
int proc_write_kFuncName(struct file *file,const char *buffer,unsigned long count,void * data)
{
	int iRet;
	if(count <= 0) { return FAIL; }
	memset(kFuncName, '\0', sizeof(kFuncName));
	iRet = copy_from_user(kFuncName, buffer, count);
	if(iRet) { return FAIL; }
	//remove '\n'
	if(kFuncName[strlen(kFuncName) - 1] == '\n')
	{
		kFuncName[strlen(kFuncName) - 1] = '\0';
	}
	dbginfo("Rcv kernel func name:%s\n",kFuncName);
	return count;
}

/*
*  
*/
int proc_read_taskInfo(char * page,char **start, off_t off, int count, int * eof,void * data)
{
	int iLen;
	iLen = sprintf(page, "%s", taskInfo);
	return iLen;
}

/*
*  
*/
int proc_write_memVal(struct file *file,const char *buffer,unsigned long count,void * data)
{
	int iRet;
	char sMemVal[MAX_LINE];
	
	if(count <= 0) { return FAIL; }
	memset(sMemVal, '\0', sizeof(sMemVal));
	iRet = copy_from_user(sMemVal, buffer, count);
	if(iRet) { return FAIL; }
	iRet = sscanf(sMemVal,"%lx",&memVal);
	if(iRet != 1) { return FAIL; }
	dbginfo("Rcv memVal:0x%lx\n",memVal);
	return count;
}

/*
*  
*/
int proc_read_memVal(char * page,char **start, off_t off, int count, int * eof,void * data)
{
	int iLen;
	iLen = sprintf(page, "%lx", memVal);
	return iLen;
}

/*
*  init memory fault injection module
*/
static int __init initME(void)
{
	dir = proc_mkdir("memoryEngine", NULL);
	if(dir == NULL)
	{
		dbginfo("Can't create /proc/memoryEngine/\n");
		return FAIL;
	}
	//dir->owner = THIS_MODULE;
	
	proc_pid = create_proc_entry("pid", PERMISSION, dir);
	if(proc_pid == NULL)
	{
		dbginfo("Can't create /proc/memoryEngine/pid\n");
		remove_proc_entry("memoryEngine", NULL);
		return FAIL;
	}
	proc_pid->write_proc = proc_write_pid;
//	proc_pid->owner = THIS_MODULE;
	
	proc_va = create_proc_entry("virtualAddr", PERMISSION, dir);
	if(proc_va == NULL)
	{
		dbginfo("Can't create /proc/memoryEngine/virtualAddr\n");
		remove_proc_entry("pid", dir);
		remove_proc_entry("memoryEngine", NULL);
		return FAIL;
	}
	proc_va->read_proc = proc_read_virtualAddr;
	proc_va->write_proc = proc_write_virtualAddr;
//	proc_va->owner = THIS_MODULE;
	
	proc_ctl = create_proc_entry("ctl", PERMISSION, dir);
	if(proc_ctl == NULL)
	{
		dbginfo("Can't create /proc/memoryEngine/ctl\n");
		remove_proc_entry("pid", dir);
		remove_proc_entry("virtualAddr", dir);
		remove_proc_entry("memoryEngine", NULL);
		return FAIL;
	}
	proc_ctl->write_proc = proc_write_ctl;
//	proc_ctl->owner = THIS_MODULE;
	
	proc_signal = create_proc_entry("signal", PERMISSION, dir);
	if(proc_signal == NULL)
	{
		dbginfo("Can't create /proc/memoryEngine/signal\n");
		remove_proc_entry("pid", dir);
		remove_proc_entry("virtualAddr", dir);
		remove_proc_entry("ctl", dir);
		remove_proc_entry("memoryEngine", NULL);
		return FAIL;
	}
	proc_signal->read_proc = proc_read_signal;
	proc_signal->write_proc = proc_write_signal;
//	proc_signal->owner = THIS_MODULE;
	
	proc_pa = create_proc_entry("physicalAddr", PERMISSION, dir);
	if(proc_pa == NULL)
	{
		dbginfo("Can't create /proc/memoryEngine/physicalAddr\n");
		remove_proc_entry("pid", dir);
		remove_proc_entry("virtualAddr", dir);
		remove_proc_entry("ctl", dir);
		remove_proc_entry("memoryEngine", NULL);
		return FAIL;
	}
	proc_pa->read_proc = proc_read_pa;
	proc_pa->write_proc = proc_write_pa;
	
	proc_kFuncName = create_proc_entry("kFuncName", PERMISSION, dir);
	if(proc_kFuncName == NULL)
	{
		dbginfo("Can't create /proc/memoryEngine/kFuncName\n");
		remove_proc_entry("pid", dir);
		remove_proc_entry("virtualAddr", dir);
		remove_proc_entry("ctl", dir);
		remove_proc_entry("physicalAddr", dir);
		remove_proc_entry("memoryEngine", NULL);
		return FAIL;
	}
	proc_kFuncName->write_proc = proc_write_kFuncName;
	
	proc_taskINfo = create_proc_entry("taskInfo", PERMISSION, dir);
	if(proc_taskINfo == NULL)
	{
		dbginfo("Can't create /proc/memoryEngine/taskInfo\n");
		remove_proc_entry("pid", dir);
		remove_proc_entry("virtualAddr", dir);
		remove_proc_entry("ctl", dir);
		remove_proc_entry("physicalAddr", dir);
		remove_proc_entry("kFuncName", dir);
		remove_proc_entry("memoryEngine", NULL);
		return FAIL;
	}
	proc_taskINfo->read_proc = proc_read_taskInfo;
	
	proc_val = create_proc_entry("memVal", PERMISSION, dir);
	if(proc_val == NULL)
	{
		dbginfo("Can't create /proc/memoryEngine/memVal\n");
		remove_proc_entry("pid", dir);
		remove_proc_entry("virtualAddr", dir);
		remove_proc_entry("ctl", dir);
		remove_proc_entry("physicalAddr", dir);
		remove_proc_entry("kFuncName", dir);
		remove_proc_entry("taskInfo", dir);
		remove_proc_entry("memoryEngine", NULL);
		return FAIL;
	}
	proc_val->write_proc = proc_write_memVal;
	proc_val->read_proc = proc_read_memVal;
	
	/*
	ret = register_jprobe(&jprobe1);
	if (ret < 0)
	{
		printk("register_jprobe jprobe1 failed, returned %d\n", ret);
		return ret;
	}
	printk("Planted jprobe at force_sig_info: %p\n", jprobe1.kp.addr);
	*/

	dbginfo("Memory engine module init\n");
	return OK;
}
/*
static int jforce_sig_info(int sig,struct siginfo *info,struct task_struct *t)
{
    printk("MemSysFI: kernel is sending signal %d to process pid: %d, comm: %s\n",sig,t->pid,t->comm);
    jprobe_return();
    return 0;
}
*/

/*
*  uninit memory fault injection module
*/
static void __exit exitME(void)
{
	remove_proc_entry("pid", dir);
	remove_proc_entry("virtualAddr", dir);
	remove_proc_entry("ctl", dir);
	remove_proc_entry("signal", dir);
	remove_proc_entry("physicalAddr", dir);
	remove_proc_entry("kFuncName", dir);
	remove_proc_entry("taskInfo", dir);
	remove_proc_entry("memVal", dir);
	remove_proc_entry("memoryEngine", NULL);
	//unregister_jprobe(&jprobe1);
	//printk("jprobe at %p unregistered.\n",	jprobe1.kp.addr);
	dbginfo("Memory engine module exit\n");
}

module_init(initME);
module_exit(exitME);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("HIT CS HDMC team");
MODULE_DESCRIPTION("Memory Engine Module.");

