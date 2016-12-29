#include <linux/module.h>
#include <linux/timer.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Satoru Takeuchi <satoru.takeuchi@gmail.com>");
MODULE_DESCRIPTION("timer kernel module: pass arg");

struct mytimer_data {
	char *name;
	int interval;
	struct timer_list timer;
};

struct mytimer_data data[2] = {
	{
		.name = "foo",
		.interval = 2,
	},
	{
		.name = "bar",
		.interval = 3,
	},
};

#define MYTIMER_TIMEOUT_SECS	10

static void mytimer_fn(unsigned long arg)
{
	struct mytimer_data *data = (struct mytimer_data *)arg;

	printk(KERN_ALERT "%s: %d secs passed.\n",
	       data->name, data->interval);
	
	mod_timer(&data->timer, jiffies + data->interval*HZ);
}

static int timer_module_init(void)
{
	int i;

	for (i = 0; i < 2; i++) {
		struct mytimer_data *d = &data[i];
		init_timer(&d->timer);
		d->timer.function = mytimer_fn;
		d->timer.expires = jiffies + d->interval*HZ;
		d->timer.data = (unsigned long)d;
		add_timer(&d->timer);
	}

	return 0;
}

static void timer_module_exit(void)
{
	int i;

	for (i = 0; i < 2; i++) {
		del_timer(&data[i].timer);
	}
}

module_init(timer_module_init);
module_exit(timer_module_exit);
