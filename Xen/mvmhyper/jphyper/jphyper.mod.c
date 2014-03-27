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
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0x1a197155, "struct_module" },
	{ 0x1a5ef3d2, "phys_to_machine_mapping" },
	{ 0x1139ffc, "max_mapnr" },
	{ 0x55526907, "xen_features" },
	{ 0x982a27e7, "hypercall_page" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0x1b9aca3f, "jprobe_return" },
	{ 0x79aa04a2, "get_random_bytes" },
	{ 0x1d26aa98, "sprintf" },
	{ 0x859204af, "sscanf" },
	{ 0xf2a644fb, "copy_from_user" },
	{ 0x1189a86a, "create_proc_entry" },
	{ 0x5017e406, "proc_mkdir" },
	{ 0x81a37453, "register_jprobe" },
	{ 0xe4b16627, "remove_proc_entry" },
	{ 0x9f3ce2a7, "unregister_jprobe" },
	{ 0x1b7d4074, "printk" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "6AB0ADC037741D90EF81ABD");
