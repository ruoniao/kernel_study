#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xa4b86400, "module_layout" },
	{ 0xa45c7b90, "stack_trace_print" },
	{ 0xdc21e866, "hrtimer_forward" },
	{ 0xa0c6befa, "hrtimer_cancel" },
	{ 0xb43f9365, "ktime_get" },
	{ 0xfbdfc558, "hrtimer_start_range_ns" },
	{ 0xc5850110, "printk" },
	{ 0xe7b00dfb, "__x86_indirect_thunk_r13" },
	{ 0xe007de41, "kallsyms_lookup_name" },
	{ 0x352b9a56, "init_task" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0x2ea2c95c, "__x86_indirect_thunk_rax" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x1ee7d3cd, "hrtimer_init" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "FF484B969AAE34C9097C984");
