
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

MODULE_DESCRIPTION("My kernel module");
MODULE_AUTHOR("%{AUTHOR} (%{EMAIL})");
MODULE_LICENSE("$LICENSE$");

static int %{APPNAMELC}_init_module(void)
{
	printk( KERN_DEBUG "Module %{APPNAME} init\n" );
	return 0;
}

static void %{APPNAMELC}_exit_module(void)
{
	printk( KERN_DEBUG "Module %{APPNAME} exit\n" );
}

module_init(%{APPNAMELC}_init_module);
module_exit(%{APPNAMELC}_exit_module);
