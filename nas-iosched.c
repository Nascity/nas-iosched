#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/rbtree.h>




static struct elevator_type nas_data = {
	.ops = {

	},

	.elevator_name = "nas-iosched",
	.elevator_alias = "nas",
	.elevator_owner = THIS_MODULE,
};

static int __init nas_init(void)
{
	return elv_register(&nas_data);
}

static void __exit nas_exit(void)
{
	elv_unregister(&nas_data);
}

module_init(nas_init);
module_exit(nas_exit);

MODULE_AUTHOR("Seung-ri Shin");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Nascity's IO scheduler");
