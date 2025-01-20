#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/rbtree.h>
#include <linux/blk-mq.h>

#define SECTORS_PER_TRACK	63
#define TRACK(__sec)		((__sec) / SECTORS_PER_TRACK)
#define RQ_TRACK(__req)		TRACK(blk_rq_pos(__req))

#define NAS_DMESG_HEADER	"[nas-iosched]"

struct nas_data {
	struct rb_root tree;
	sector_t headpos;
	spinlock_t lock;
};

static int
nas_init_queue(struct request_queue* q, struct elevator_type* e)
{
	struct nas_data* nd;
	struct elevator_queue* eq;

	eq = elevator_alloc(q, e);
	if (!eq)
		return -ENOMEM;

	nd = kzalloc_node(sizeof(struct nas_data), GFP_KERNEL, q->node);
	if (!nd)
	{
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}
	eq->elevator_data = nd;
	nd->tree = RB_ROOT;
	nd->headpos = 0;
	q->elevator = eq;

	spin_lock_init(&nd->lock);

	printk(NAS_DMESG_HEADER"nas-iosched initialized.\n");
	return 0;
}

static void
nas_exit_queue(struct elevator_queue* e)
{
	struct nas_data* nd = e->elevator_data;

	kfree(nd);

	printk(NAS_DMESG_HEADER"nas-iosched exited.\n");
}

static void
nas_insert_request(struct nas_data* nd, struct request* rq, bool at_head)
{
	elv_rb_add(&nd->tree, rq);
	printk(NAS_DMESG_HEADER "Request for sector %llu with size %u inserted.\n",
			blk_rq_pos(rq), blk_rq_bytes(rq));
}

static void
nas_insert_requests(struct blk_mq_hw_ctx* hctx,
		struct list_head* list, bool at_head)
{
	struct request_queue* q = hctx->queue;
	struct nas_data* nd = q->elevator->elevator_data;
	struct request* rq;

	spin_lock(&nd->lock);
	while (!list_empty(list))
	{
		rq = list_first_entry(list, struct request, queuelist);
		list_del_init(&rq->queuelist);
		nas_insert_request(nd, rq, at_head);
	}
	spin_unlock(&nd->lock);
}

struct request*
__nas_dispatch_requests(struct nas_data* nd)
{
	struct rb_node* p = nd->tree.rb_node;
	struct request* rq = NULL;

	while (p)
	{
		rq = rb_entry(p, struct request, rb_node);

		if (RQ_TRACK(rq) == TRACK(nd->headpos))
			break;
		else if (RQ_TRACK(rq) > TRACK(nd->headpos))
			p = p->rb_right;
		else
			p = p->rb_left;
	}
	if (rq)
		elv_rb_del(nd->tree, rq);

	return rq;
}

struct request*
nas_dispatch_requests(struct blk_mq_hw_ctx* hctx)
{
	struct nas_data* nd = hctx->queue->elevator->elevator_data;
	struct request* rq;
	
	spin_lock(&nd->lock);
	rq = __nas_dispatch_requests(nd);
	printk(NAS_DMESG_HEADER "Sector %llu dispatched.\n", blk_rq_pos(rq));
	spin_unlock(&nd->lock);

	return rq;
}

bool
nas_has_work(struct blk_mq_hw_ctx* hctx)
{
	struct nas_data* nd = hctx->queue->elevator->elevator_data;
	
	return nd->tree.rb_node != NULL;
}

static struct elevator_type nas = {
	.ops = {
		// init and cleanup
		.init_sched		= nas_init_queue,
		.exit_sched		= nas_exit_queue,

		// request insertion and dispatch
		.insert_requests	= nas_insert_requests,
		.dispatch_request	= nas_dispatch_requests,

		.has_work		= nas_has_work,
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
