# 系统调用小demo学习
`通过修改内核代码监控open fork 等系统调用，通过自定义系统调用从用户控件获取数据
`
## 1，从系统调用表添加自定义系统调用号
`sudo vim arch/x86/entry/syscalls/syscall_64.tbl
`

添加: 335  common  myaudit __x64_sys_myaudit
## 2, 定义的系统调用.c 文件并在Makefile添加编译选项
1. 拷贝myaudit.c 到 arch/x86/kernel/ 下
2. 在 arch/x86/kernel/Makefile 添加 
`obj-y                   += myaudit.o
` 
### 注意EXPORT_SYMBOL(函数名) 宏的使用
- 作用：对于EXPORT_SYMBOL标签内的函数或符号对所有内核代码公开，可以不通过修改内核代码就可在内核模块中使用，即使用EXPORT_SYMBOL可以将一个函数以符号的方式导出提供给其他模块使用

- 使用：
    1. 在模块函数定义之后使用“EXPORT_SYMBOL（函数名）”来声明。
    2. 在调用该函数的另外一个模块中使用extern对之声明。
    3. 先加载定义该函数的模块，然后再加载调用该函数的模块，请注意这个先后顺序。

## 3，在Linux syscall interfaces 文件中添加存数据和读数据的钩子函数声明(添加接口声明)

在include/linux/syscalls.h 系统调用中声明监控函数存数据函数和系统调用时读取数据函数
`sudo vim include/linux/syscalls.h
`
添加文件最后添加以下函数声明（#endif 前）
```
asmlinkage long sys_myaudit(u8,u8 *,u16,u8);
extern void (*my_audit)(int,int);
```
### 注意asmlinkage宏的使用
- 作用：大多适用于系统调用；告诉编译器此函数声明是通过stack 传参，而不是寄存器传参。在x86中所有的系统调用函数，是先将参数压入stack 后调用 sys_* 函数来传参，需要asmlinkage声明。

## 4, 在总系统调用函数入口中添加拦截函数
在 arch/x86/entry/common.c do_syscall_64函数中
```
if (likely(nr < NR_syscalls)) {
                nr = array_index_nospec(nr, NR_syscalls);
                regs->ax = sys_call_table[nr](regs);

                //add myself code
                if( nr==2 || nr==3 || nr==39 || nr==56 || nr== 57 || nr==59){
                        if(my_audit){
                                (*my_audit)(nr,regs->ax);
                        }else{
                                printk("my_audit is not exist");
                        }
                }
#ifdef CONFIG_X86_X32_ABI
        } 
```
## 5, 使用oldconfig 编译5.5版本
1. 将/usr/src/linux-`uname-r` 下的.config copy 到源码目录下
2. sudo make menuconfig load 之前的config并保存
3. make 
4. sudo make modules_install
5. sudo make install
6. sudo reboot
### 编译遇到错误 
- error: Makefile:956: "Cannot use CONFIG_STACK_VALIDATION=y, please install libelf-dev, libelf-devel or elfutils-libelf-devel
- 解决：apt install libelf-dev 有个依赖需要安装，根据提示安装一下
- error:No rule to make target ‘debian/canonical-certs.pem‘, needed by ‘certs/x509_certificate_list‘
- 解决：编辑.config 修改为 CONFIG_SYSTEM_TRUSTED_KEYS=""
## 6, 编写系统内核模块实现上述两个钩子函数并编译加载
- make && insmod my_audit.ko
## 7, 使用用户态 c程序调用自定义的系统调用测试
- gcc test_syscall.c -o test_syscall
- ./test_syscall