#include <linux/kernel.h>
#include "linux/sched.h"
#include "sched/sched.h"




asmlinkage long sys_hello(void) {
	printk("Hello, World!");
	return 0;

}

asmlinkage long get_vruntime(void) {
    struct rq *rq = this_rq();
    struct task_struct *t = rq->curr;
    return t->se.vruntime;
}
