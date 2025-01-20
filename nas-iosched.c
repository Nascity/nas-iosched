#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/rbtree.h>

struct nas_data {
	struct rb_root sort_list[2];
	struct list_head dispatch;
	spinlock_t lock;
}

static inline struct rb_root*
nas_rb_root(struct nas_data* nd, struct request* rq)
{
	return &nd->sort_list[rq_data_dir(rq)];
}

static int
nas_init_queue(struct request_queue* q, struct elevator_type* e)
{
	struct nas_data* nd;
	struct elevator_queue* eq;

	eq = elevator_alloc(q, e);
	if (!eq)
		return -ENOMEM;

	nd = kzalloc_node(sizeof(struct nas_data), GFP_KERNEL, q->node);
	if (!dd)
	{
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}
	eq->elevator_data = dd;

	dd->sort_list[READ] = RB_ROOT;
	dd->sort_list[WRITE] = RB_ROOT;

	spin_lock_init(&dd->lock);

	q->elevator = eq;
	return 0;
}

static void
nas_exit_queue(struct elevator_queue* e)
{
	struct nas_data* nd = e->elevator_data;

	kfree(nd);
}

static struct elevator_type nas = {
	.ops = {
		.insert_requests	= nas_insert_requests,
		.dispatch_requests	= nas_dispatch_requests,
		.prepare_requests	= nas_prepare_requests,
		.next_request		= elv_rb_latter_request,
		.former_request		= elv_rb_former_request,
		.bio_merge		= nas_bio_merge,
		.request_merge		= nas_request_merge,
		.init_sched		= nas_init_queue,
		.exit_sched		= nas_exit_queue,
	},
	.elevator_name = "nas-iosched",
	.elevator_alias = "nas",
	.elevator_owner = THIS_MODULE,
};

static int __init nas_init(void)
{
	return elv_register(&nas);
}

static void __exit nas_exit(void)
{
	elv_unregister(&nas);
}

module_init(nas_init);
module_exit(nas_exit);

MODULE_AUTHOR("Seung-ri Shin");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Nascity's IO scheduler");
