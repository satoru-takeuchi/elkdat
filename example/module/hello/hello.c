#include <linux/module.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Satoru Takeuchi <satoru.takeuchi@gmail.com>");
MODULE_DESCRIPTION("Hello world kernel module");

static int hello_init(void) {
	printk(KERN_ALERT "Hello world!\n");
	return 0;
}

static void hello_exit(void) {
	printk(KERN_ALERT "driver unloaded\n");
}

module_init(hello_init);
module_exit(hello_exit);
