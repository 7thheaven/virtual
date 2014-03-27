#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xb317a51, "module_layout" },
	{ 0x45ddfb23, "per_cpu__current_task" },
	{ 0xd0d8621b, "strlen" },
	{ 0x3841ab01, "unregister_kprobe" },
	{ 0x79aa04a2, "get_random_bytes" },
	{ 0xed882c9a, "malloc_sizes" },
	{ 0xdc714560, "register_kprobe" },
	{ 0x105e2727, "__tracepoint_kmalloc" },
	{ 0xdd902606, "down_interruptible" },
	{ 0xfc6d1e40, "remove_proc_entry" },
	{ 0x1b9aca3f, "jprobe_return" },
	{ 0x3c2c5af5, "sprintf" },
	{ 0xb5d8913d, "register_jprobe" },
	{ 0xea619b76, "proc_mkdir" },
	{ 0xb72397d5, "printk" },
	{ 0x42224298, "sscanf" },
	{ 0xb4390f9a, "mcount" },
	{ 0xa076d470, "init_task" },
	{ 0xdd544492, "kmem_cache_alloc" },
	{ 0x7285f84f, "unregister_jprobe" },
	{ 0x13da54af, "create_proc_entry" },
	{ 0x37a0cba, "kfree" },
	{ 0x22a2e20b, "up" },
	{ 0xf2a644fb, "copy_from_user" },
	{ 0xe914e41e, "strcpy" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "C3639217BBCC2FDD3C6CE7E");
