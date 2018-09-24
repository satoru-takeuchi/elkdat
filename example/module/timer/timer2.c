#include <linux/module.h>
#include <linux/timer.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Satoru Takeuchi <satoru.takeuchi@gmail.com>");
MODULE_DESCRIPTION("timer kernel module: with critical bug");

struct timer_list mytimer;

#define MYTIMER_TIMEOUT_SECS	10

static void mytimer_fn(struct timer_list *timer)
{
	printk(KERN_ALERT "10 secs passed.\n");
}

static int mymodule_init(void)
{
	timer_setup(&mytimer, mytimer_fn, 0);
	mytimer.expires = jiffies + MYTIMER_TIMEOUT_SECS*HZ;
	add_timer(&mytimer);

	return 0;
}

static void mymodule_exit(void)
{
	/* do nothing */
}

module_init(mymodule_init);
module_exit(mymodule_exit);
