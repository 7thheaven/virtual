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
	{ 0x3ee649bc, "machine_to_phys_order" },
	{ 0xd0d8621b, "strlen" },
	{ 0x3841ab01, "unregister_kprobe" },
	{ 0x55526907, "xen_features" },
	{ 0x6226b9fa, "machine_to_phys_mapping" },
	{ 0xdc714560, "register_kprobe" },
	{ 0xfc6d1e40, "remove_proc_entry" },
	{ 0x3c2c5af5, "sprintf" },
	{ 0x1139ffc, "max_mapnr" },
	{ 0xe2d5255a, "strcmp" },
	{ 0xea619b76, "proc_mkdir" },
	{ 0xb72397d5, "printk" },
	{ 0x42224298, "sscanf" },
	{ 0xb4390f9a, "mcount" },
	{ 0xa076d470, "init_task" },
	{ 0x13da54af, "create_proc_entry" },
	{ 0x701d0ebd, "snprintf" },
	{ 0xf2a644fb, "copy_from_user" },
	{ 0xe914e41e, "strcpy" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "38E466F5A428D7012397355");
