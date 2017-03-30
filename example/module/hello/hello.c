#include <linux/module.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Satoru Takeuchi <satoru.takeuchi@gmail.com>");
MODULE_DESCRIPTION("Hello world kernel module");

static int mymodule_init(void) {
	printk(KERN_ALERT "Hello world!\n");
	return 0;
}

static void mymodule_exit(void) {
	/* Do nothing */
}

module_init(mymodule_init);
module_exit(mymodule_exit);
