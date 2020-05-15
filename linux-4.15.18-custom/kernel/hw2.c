#include <linux/kernel.h>
asmlinkage long sys_hello(void) {
	printk("Hello, World!");
	return 0;
}
