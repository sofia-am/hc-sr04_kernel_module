#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
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
__used __section("__versions") = {
	{ 0x7c30cdd7, "module_layout" },
	{ 0xe247e3c3, "cdev_del" },
	{ 0x9dfdf722, "gpio_free_array" },
	{ 0x8574ca6c, "gpio_request_array" },
	{ 0x9d4cb3b3, "gpiod_direction_output_raw" },
	{ 0xfe990052, "gpio_free" },
	{ 0x47229b5c, "gpio_request" },
	{ 0xc69506dc, "class_destroy" },
	{ 0xe3a719e1, "device_destroy" },
	{ 0xe50f8328, "cdev_add" },
	{ 0x322b4fd3, "cdev_init" },
	{ 0x2ea9c462, "device_create" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x75d7cdf1, "__class_create" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x5be8f50f, "kmem_cache_alloc_trace" },
	{ 0xed16b9eb, "kmalloc_caches" },
	{ 0x97255bdf, "strlen" },
	{ 0x51a910c0, "arm_copy_to_user" },
	{ 0xb0592d98, "gpiod_set_raw_value" },
	{ 0xaf78c0b, "gpiod_get_raw_value" },
	{ 0xa32498f0, "gpio_to_desc" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x5f754e5a, "memset" },
	{ 0xae353d77, "arm_copy_from_user" },
	{ 0x2cfde9a2, "warn_slowpath_fmt" },
	{ 0x92997ed8, "_printk" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "7F6F3C424A3B695B0D5B55D");
