#include<linux/miscdevice.h>
#include<linux/module.h>
#include<linux/fs.h>
#include <linux/kprobes.h>
#include "debug.h"
#include "fiber_methods.h"

#define NAME "fibers"

int init_module(void);
void cleanup_module(void);
static int hit_open (struct inode *, struct file *);
static int hit_release(struct inode *, struct file *);
static long hit_ioctl(struct file *, unsigned int, unsigned long);
