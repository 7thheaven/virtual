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
	{ 0x79aa04a2, "get_random_bytes" },
	{ 0x1b9aca3f, "jprobe_return" },
	{ 0xb5d8913d, "register_jprobe" },
	{ 0xb72397d5, "printk" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0xb4390f9a, "mcount" },
	{ 0x7285f84f, "unregister_jprobe" },
	{ 0xf2a644fb, "copy_from_user" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "C648E7FC8116CA8A7EA3FCA");
