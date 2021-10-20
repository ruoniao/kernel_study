#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/nospec.h>

#define MAX_SYMBOL_LEN    64
#define COMM_SIZE 16
#define AUDIT_BUF_SIZE 1

static char symbol[MAX_SYMBOL_LEN] = "do_syscall_64";
module_param_string(symbol, symbol, sizeof(symbol), 0644);

/* For each probe you need to allocate a kprobe structure */
// 定义一个实例kp并初始化symbol_name为"_do_fork"，将探测_do_fork函数。
static struct kprobe kp = {
    .symbol_name    = symbol,
};

struct syscall_buf{
u32 serial;
u64 ts_sec;
u64 ts_micro;
u32 syscall;
u32 status;
pid_t pid;
uid_t uid;
u8 comm[COMM_SIZE];	
};

static struct  syscall_buf  audit_buf[AUDIT_BUF_SIZE];
static int current_pos = 0;
static u32 serial = 0;

// 封装系统调用
void syscall_package(int syscall,int return_status)
{	
	struct syscall_buf *ppb_temp;
	struct timespec64 nowtime;
	
	ktime_get_real_ts64(&nowtime);
	if(current_pos<AUDIT_BUF_SIZE){
		ppb_temp = &audit_buf[current_pos];
		ppb_temp->serial = serial++;
		ppb_temp->ts_sec = nowtime.tv_sec;
		ppb_temp->ts_micro = nowtime.tv_nsec;
		ppb_temp->syscall = syscall;
		ppb_temp->status = return_status;
		ppb_temp->pid = current->pid;
		ppb_temp->uid = current->tgid; 
		
		memcpy(ppb_temp->comm,current->comm,COMM_SIZE);
		if(++current_pos ==AUDIT_BUF_SIZE)
			{
				pr_info("IN MODULE_AUDIT:yes,it is full\n");
			}
		}
	
}
// 解绑打印系统调用信息
void syscall_unpackage(void){
    struct syscall_buf *ppb_temp;
    if(current_pos==AUDIT_BUF_SIZE){
        ppb_temp = audit_buf;
        pr_info("serial:%d,\t syscall :%d,\t pid:%d,\t comm:%s,\t ",
        ppb_temp->serial,ppb_temp->syscall,ppb_temp->pid,ppb_temp->comm);
    }
}

/* kprobe pre_handler: called just before the probed instruction is executed */
static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
#ifdef CONFIG_X86
    // pr_info("<%s> pre_handler: p->addr = %pF, ip = %lx, flags = 0x%lx\n",
    //     p->symbol_name, p->addr, regs->ip, regs->flags);
    unsigned long nr = regs->orig_ax;
    syscall_package(nr,0);
#endif

    /* A dump_stack() here will give a stack backtrace */
    return 0;
}

/* kprobe post_handler: called after the probed instruction is executed */
static void handler_post(struct kprobe *p, struct pt_regs *regs,
                unsigned long flags)
{
#ifdef CONFIG_X86
    // pr_info("<%s> post_handler: p->addr = %pF, flags = 0x%lx\n",
    //     p->symbol_name, p->addr, regs->flags);
    syscall_unpackage();
#endif
}

/*
 * fault_handler: this is called if an exception is generated for any
 * instruction within the pre- or post-handler, or when Kprobes
 * single-steps the probed instruction.
 */
static int handler_fault(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
    pr_info("fault_handler: p->addr = %pF, trap #%dn", p->addr, trapnr);
    /* Return 0 because we don't handle the fault. */
    return 0;
}

static int __init kprobe_init(void)
{
    int ret;
    //初始化kp的三个回调函数。
    kp.pre_handler = handler_pre;
    kp.post_handler = handler_post;
    kp.fault_handler = handler_fault;
    //注册kp探测点到内核。
    ret = register_kprobe(&kp);
    if (ret < 0) {
        pr_err("register_kprobe failed, returned %d\n", ret);
        return ret;
    }
    pr_info("Planted kprobe at %pF\n", kp.addr);
    return 0;
}

static void __exit kprobe_exit(void)
{
    unregister_kprobe(&kp);
    pr_info("kprobe at %pF unregistered\n", kp.addr);
}

module_init(kprobe_init)
module_exit(kprobe_exit)
MODULE_LICENSE("GPL");
