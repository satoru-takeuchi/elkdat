#include <linux/module.h>
#include <linux/timer.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Satoru Takeuchi <satoru.takeuchi@gmail.com>");
MODULE_DESCRIPTION("timer kernel module: with critical bug");

struct timer_list mytimer;

#define MYTIMER_TIMEOUT_SECS	10

static void mytimer_fn(unsigned long arg)
{
	printk(KERN_ALERT "10 secs passed.\n");
}

static int timer_module_init(void)
{
	init_timer(&mytimer);
	mytimer.expires = jiffies + MYTIMER_TIMEOUT_SECS*HZ;
	mytimer.data = 0;
	mytimer.function = mytimer_fn;
	add_timer(&mytimer);

	return 0;
}

static void timer_module_exit(void)
{
	/* do nothing */
}

module_init(timer_module_init);
module_exit(timer_module_exit);
