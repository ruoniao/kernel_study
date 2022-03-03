#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/sched.h>
#include <linux/export.h>
#include <linux/delay.h>


// 无符号长整型寄存器 cr0 和 cr3
static unsigned long cr0,cr3;
// 声明并赋值一个无符号长整形 vaddr=0
static unsigned long vaddr = 0;

// 获取并打印页目录，页4级目录地址，页上级目录地址，页中级目录地址，页表地址，偏移地址
static void get_pgtable_macro(void){
    // 内核函数，获取cr0 cr3寄存器的值
    cr0 = read_cr0();
    cr3 = read_cr3_pa();
    printk("cr0 = 0x%lx, cr3 = 0x%lx\n",cr0,cr3);

    printk("PGDIR_SHIFT = %d\n",PGDIR_SHIFT); // 获取页目录 所能映射区域大小的对数
    printk("P4D_SHIFT = %d\n",P4D_SHIFT); // 页4级目录 所能映射区域大小的对数
    printk("PUD_SHIFT = %d\n",PUD_SHIFT); // 页上级目录 所能映射区域大小的对数
    printk("PMD_SHIFT = %d\n",PMD_SHIFT); // 页中级目录 所能映射区域大小的对数
    printk("PAGE_SHIFT = %d\n",PAGE_SHIFT); // 页掩码，因为是4K 对其，所以低12位没有使用，可以屏蔽

    printk("PTRS_PER_PGD = %d\n",PTRS_PER_PGD); // 获取页目录项个数，因为PGD是（39-47）9位,所以个数是2^9=512个
    printk("PTRS_PER_P4D = %d\n",PTRS_PER_P4D); // 获取 页4级目录地址 个数，因为P4D linux 没有使用，所以是1 2^0=1
    printk("PTRS_PER_PUD = %d\n",PTRS_PER_PUD); // 获取 页上级目录地址 个数，因为PUD是（30-38）9位,所以个数是2^9=512个
    printk("PTRS_PER_PMD = %d\n",PTRS_PER_PMD); // 获取 页中级目录地址 个数，因为PMD是（21-29）9位,所以个数是2^9=512个
    printk("PTRS_PER_PTE = %d\n",PTRS_PER_PTE); // 获取 页表地址 个数，因为PTE是(12-20）9位,所以个数是2^9=512个
    /*
     *   cr0 = 0x80050033, cr3 = 0x7b818000
         PGDIR_SHIFT = 39
         P4D_SHIFT = 39
         PUD_SHIFT = 30
         PMD_SHIFT = 21
         PAGE_SHIFT = 12
         PTRS_PER_PGD = 512
         PTRS_PER_P4D = 1
         PTRS_PER_PUD = 512
         PTRS_PER_PMD = 512
         PTRS_PER_PTE = 512
         PAGE_MASK = 0xfffffffffffff000
     * */

}

// 虚拟地址转为物理地址
static unsigned long vaddr2paddr(unsigned long vaddr){
    return 0;
}

// 模块入口函数

static int __init v2p_init(void){
    get_pgtable_macro();
    return 0;
}

// 模块退出函数
static void __exit v2p_exit(void)
{
    return;
}

module_init(v2p_init);
module_exit(v2p_exit);
MODULE_LICENSE("GPL");

