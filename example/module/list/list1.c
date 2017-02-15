#include <linux/module.h>
#include <linux/slab.h>
#include <linux/list.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Satoru Takeuchi <satoru.takeuchi@gmail.com>");
MODULE_DESCRIPTION("an example of list data structure");

static LIST_HEAD(mylist);

struct mylist_entry {
	struct list_head list;
	int n;
};

static void mylist_add(int n) {
	struct mylist_entry *e = kmalloc(sizeof(*e), GFP_KERNEL);
	e->n = n;
	list_add(&e->list, &mylist);
	printk(KERN_ALERT "mylist: %d is added to the head\n", n);
}

static void mylist_add_tail(int n) {
	struct mylist_entry *e = kmalloc(sizeof(*e), GFP_KERNEL);
	e->n = n;
	list_add_tail(&e->list, &mylist);
	printk(KERN_ALERT "mylist: %d is added to the head\n", n);
}

static void mylist_del_head(void) {
	struct mylist_entry *e = list_first_entry(&mylist, struct mylist_entry, list);
	int n = e->n;
	list_del(&e->list);
	kfree(e);
	printk(KERN_ALERT "mylist: %d is deleted from the head\n", n);
}

static void mylist_show(void) {
	struct mylist_entry *e;

	printk(KERN_ALERT "mylist: show contents\n");

	list_for_each_entry(e, &mylist, list) {
		printk(KERN_ALERT "\t%d\n", e->n);
	}
}

static int mymodule_init(void) {
	mylist_show();
	mylist_add(1);
	mylist_show();
	mylist_add(2);
	mylist_show();
	mylist_add(3);
	mylist_show();
	mylist_del_head();
	mylist_show();
	mylist_del_head();
	mylist_show();
	mylist_add_tail(4);
	mylist_show();
	mylist_del_head();
	mylist_show();
	mylist_del_head();
	mylist_show();

	return 0;
}

static void mymodule_exit(void) {
	/* Do nothing */
}

module_init(mymodule_init);
module_exit(mymodule_exit);
