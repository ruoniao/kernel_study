/**
 * Boyou Xie's load monitor module
 *
 * Copyright (C) 2018 Baoyou Xie.
 *
 * Author: Baoyou Xie <baoyou.xie@gmail.com>
 *
 * License terms: GNU General Public License (GPL) version 2
 */
#include "load.h"
#include <linux/version.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/kallsyms.h>
#include <linux/sched.h>
#include <linux/tracepoint.h>
#include <linux/stacktrace.h>
#include <linux/sched/task.h> /*init_task 头文件*/
#include <linux/sched/signal.h> /*do_each_thread 头文件*/
#include <asm/ptrace.h>


struct hrtimer timer;
static unsigned long *ptr_avenrun;
/*

 // 根据源代码 https://elixir.bootlin.com/linux/v5.4.80/source/include/linux/stacktrace.h#L64 可以知道，不取消定义不会进行定义
 //  stack_trace 预编译就不会通过，报错
 // 走过的坑


 采坑历程：
 0, 错误日志： error: storage size of ‘trace’ isn’t known
    问题原因: 没有取消一个宏的定义
    解决办法：
        根据源代码 https://elixir.bootlin.com/linux/v5.4.80/source/include/linux/stacktrace.h#L64 可以知道，不取消定义不会进行定义
        stack_trace 预编译就不会通过，报错
         // 1, 不知道c的编译过程，两眼抹黑
         // 2, 知道了找不到这个结构体，但不了解宏的概念
         // 3, 了解了宏之后，拿着5.4.80 的模块代码去找 4.9的源码 去定义宏
         // 4, 定义好了宏，定义的位置在include 之后，相当于没有定义，后拉才定义到 load.h 后再先include load.h
         // 5, 最终解决
 1, 错误日志：stack_trace_print 找不到该函数
    问题原因：拿着4.9 kernel 的函数名在5.6.80 的模块中使用， 能编译通过才怪。
    解决办法： 悬崖勒马。用 5.6.80 的相关函数替换之前的4.9的功能函数

 2, 错误日志：error: ‘ptr_avenrun’ undeclared (first use in this function); did you mean ‘pr_alert’?
    错误原因：ptr_avenrun 没有 预先全局声明，编译报错
    解决办法： 仔细看教学视频； 发现每个使用这个变量的都没有定义，经视频确认缺少全局定义

 3, 错误日志：编译没有语法错误，但是 报 stack_trace_save_tsk **.ko undefined !
    问题原因：Linux 内核为了减少命名空间的污染，并做到正确的信息隐藏，内核提供了管理内核符号可见性的方法，没有被 EXPORT_SYMBOL 相关的宏导出的变量或函数是不能直接使用的
    解决办法： 通过函数名获取函数的指针，通过函数指针调用函数；
    弯路： 以为和之前的宏一样没有定义，想在模块编译中加入KConfig 参数，后来知道make 编译模块是 obj-o 只能指定模块, 指定obj-$(CONFIG)依赖源码，感觉方向不对；
    后来想这个 编译 obj-m +=  加入 stack_trace module ，但是没有这个module ，感觉方向也错了。
    后来灵光一闪 ，获取系统 负载值的 avenrun 函数调用 是通过 获取指针调用的，此方法应该也是

 知识点:
    1, define undefine 宏定义 本质是字符串的替换
    2, 数组的名称 也就是 数组的指针地址 也就是 数组的首个元素的地址，参数传递可以直接传递 数组名
    3, typedef 是 类型的改名；
    4, typedef 函数指针类型定义，通过函数指针调用函数的方法
        typedef 重新定义，定义一个 返回值为无符号的整形，的函数型指针，类型名(注意不是变量名)
        不要再犯 拿着类型名称 去 =  赋值了 如  pfun  = (void *)kallsyms_lookup_name("stack_trace_save_tsk");
        是pfun，类型是指针，后面有4个参数,参数来源于stack_trace_save_tsk 函数
    5, 根据函数名获取函数指针的4种方式
          该函数没有进行export 需要根据函数名获取函数指针进行调用（有四种方式）
         1,https://www.cnblogs.com/sky-heaven/p/5192778.html
         2,https://www.heapdump.cn/article/524745
         举例： pfun f1 = (void *)kallsyms_lookup_name("stack_trace_save_tsk");
    6, 没有kallsyms_lookup_name 的编译器版本可以直接通过命令获取地址使用 如:
        root@ruoniao:~/CLionProjects/kernel_study/load_monitor# cat /proc/kallsyms | grep tack_trace_save_tsk
        ffffffff8cd2f7e0 T stack_trace_save_tsk
        pfun f1 = 0xffffffffb2d2f7e0;
        https://zhuanlan.zhihu.com/p/28461589

    7, make Kconfig 的层级结构
       make 的三种编译方式 1，直接编译 obj-y 2, 按配置文件 obj-$(CONF_FILE) 3,按模块 obj-m +=
       Kconfig 的 文件格式

    8, 指针也分类型 int *p 表示 int 类型的指针变量;  p 是 指针变量的值，也就是地址 ，*p 表示 取值操作，取该地址的值；与&取地址操作相反
    另外注意 函数参数的类型 是指针类型还是 unsiged 无符号类型

    9
        1, sizeof 取大小操作
        2, 模块中有多个.c 文件怎么编写 Makefile

    10: 思考：
        1, 该代码还有哪些改进空间，
        2, 还有什么方法可以跟踪 load 高的问题，方法有何优缺点；
        3, I/O 密集操作之后 wa 的值还是为0 为何？ wa 通过top 观看

*/
typedef unsigned int (*pfun)(struct task_struct *,
                     unsigned long *, unsigned int ,
                     unsigned int );

#define FSHIFT		11		/* nr of bits of precision */
#define FIXED_1		(1<<FSHIFT)	/* 1.0 as fixed-point */
#define LOAD_INT(x) ((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1-1)) * 100)
#define BACKTRACE_DEPTH 20

/*#if defined(UBUNTU_1804)
extern struct task_struct init_task;
#define next_task(p) \
	list_entry_rcu((p)->tasks.next, struct task_struct, tasks)
#define do_each_thread(g, t) \
	for (g = t = &init_task ; (g = t = next_task(g)) != &init_task ; ) do
#define while_each_thread(g, t) \
	while ((t = next_thread(t)) != g)
static inline struct task_struct *next_thread(const struct task_struct *p)
{
	return list_entry_rcu(p->thread_group.next,
			      struct task_struct, thread_group);
}
#endif */


static void print_all_task_stack(void)
{
	struct task_struct *g, *p;
	unsigned long backtrace[BACKTRACE_DEPTH];
	struct stack_trace trace;

    //pfun = (void*)kallsyms_lookup_name("stack_trace_save_tsk");
    //pfun f1 = 0xffffffffb2d2f7e0;
    pfun f1 = (void *)kallsyms_lookup_name("stack_trace_save_tsk");
	memset(&trace, 0, sizeof(trace));
	memset(backtrace, 0, BACKTRACE_DEPTH * sizeof(unsigned long));
	trace.max_entries = BACKTRACE_DEPTH;
	trace.entries = backtrace;

    printk("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printk("\tLoad: %lu.%02lu, %lu.%02lu, %lu.%02lu\n",
    LOAD_INT(ptr_avenrun[0]), LOAD_FRAC(ptr_avenrun[0]),
    LOAD_INT(ptr_avenrun[1]), LOAD_FRAC(ptr_avenrun[1]),
    LOAD_INT(ptr_avenrun[2]), LOAD_FRAC(ptr_avenrun[2]));
    printk("dump all task: balabala\n");

    rcu_read_lock();

	printk("dump running task.\n");
	do_each_thread(g, p) {
		if (p->state == TASK_RUNNING) {
			printk("running task, comm: %s, pid %d\n",
				p->comm, p->pid);
			memset(&trace, 0, sizeof(trace));
			memset(backtrace, 0, BACKTRACE_DEPTH * sizeof(unsigned long));
			trace.max_entries = BACKTRACE_DEPTH;
			trace.entries = backtrace;
            f1(p, backtrace, BACKTRACE_DEPTH,0);
            stack_trace_print(backtrace, BACKTRACE_DEPTH, 0);
		}
	} while_each_thread(g, p);

	printk("dump uninterrupted task.\n");
	do_each_thread(g, p) {
		if (p->state & TASK_UNINTERRUPTIBLE) {
			printk("uninterrupted task, comm: %s, pid %d\n",
				p->comm, p->pid);
			memset(&trace, 0, sizeof(trace));
			memset(backtrace, 0, BACKTRACE_DEPTH * sizeof(unsigned long));
			trace.max_entries = BACKTRACE_DEPTH;
			trace.entries = backtrace;
            f1(p, backtrace, BACKTRACE_DEPTH,0);
            stack_trace_print(backtrace, BACKTRACE_DEPTH, 0);
		}
	} while_each_thread(g, p);

	rcu_read_unlock();
}

static void check_load(void)
{
	static ktime_t last;
	u64 ms;
	//int load = LOAD_INT(ptr_avenrun[0]); /* 最近1分钟的Load值 */
	//if (load < 3)
	//	return;

	/**
	 * 如果上次打印时间与当前时间相差不到20秒，就直接退出
	 */
	ms = ktime_to_ms(ktime_sub(ktime_get(), last));
	if (ms < 20 * 1000)
		return;

	last = ktime_get();
	print_all_task_stack();
}

static enum hrtimer_restart monitor_handler(struct hrtimer *hrtimer)
{
	enum hrtimer_restart ret = HRTIMER_RESTART;

	check_load();

	hrtimer_forward_now(hrtimer, ms_to_ktime(10));

	return ret;
}

static void start_timer(void)
{
	hrtimer_init(&timer, CLOCK_MONOTONIC, HRTIMER_MODE_PINNED);
	timer.function = monitor_handler;
	hrtimer_start_range_ns(&timer, ms_to_ktime(10),	0, HRTIMER_MODE_REL_PINNED);
}

static int load_monitor_init(void)
{
    ptr_avenrun = (void *)kallsyms_lookup_name("avenrun");
    if (!ptr_avenrun)
        return -EINVAL;
    //printk("CONFIG_STACKTRACE: %d",CONFIG_STACKTRACE );
    //printk("CONFIG_ARCH_STACKWALK: %d",CONFIG_ARCH_STACKWALK);
    start_timer();
    printk("load-monitor loaded.\n");
    return 0;
}

static void load_monitor_exit(void)
{
	hrtimer_cancel(&timer);

        printk("load-monitor unloaded.\n");
}

module_init(load_monitor_init)
module_exit(load_monitor_exit)

MODULE_DESCRIPTION("Baoyou Xie's load monitor module");
MODULE_AUTHOR("Baoyou Xie <baoyou.xie@gmail.com>");
MODULE_LICENSE("GPL v2");
