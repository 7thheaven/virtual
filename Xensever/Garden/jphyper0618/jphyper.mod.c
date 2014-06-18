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
	{ 0xf76a4ab8, "hypercall_page" },
	{ 0x45ddfb23, "per_cpu__current_task" },
	{ 0x1a5ef3d2, "phys_to_machine_mapping" },
	{ 0x79aa04a2, "get_random_bytes" },
	{ 0x55526907, "xen_features" },
	{ 0xfc6d1e40, "remove_proc_entry" },
	{ 0x1b9aca3f, "jprobe_return" },
	{ 0x3c2c5af5, "sprintf" },
	{ 0x1139ffc, "max_mapnr" },
	{ 0xb5d8913d, "register_jprobe" },
	{ 0xea619b76, "proc_mkdir" },
	{ 0xb72397d5, "printk" },
	{ 0x42224298, "sscanf" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0xb4390f9a, "mcount" },
	{ 0x7285f84f, "unregister_jprobe" },
	{ 0x13da54af, "create_proc_entry" },
	{ 0xf2a644fb, "copy_from_user" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "28134D6AAF9B2FA3F0561EE");
