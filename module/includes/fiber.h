#include<linux/miscdevice.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/kprobes.h>
#include"debug.h"
#include"fiber_methods.h"
#include"proc.h"

#define NAME "fibers"


int init_module(void);
void cleanup_module(void);
static int hit_open (struct inode *, struct file *);
static int hit_release(struct inode *, struct file *);
static long hit_ioctl(struct file *, unsigned int, unsigned long);


static struct file_operations fops = {

  .owner = THIS_MODULE,      //macro for the current module
  .open = hit_open,
  .release = hit_release,
  .unlocked_ioctl = hit_ioctl
};


static struct miscdevice mdev = {

  .minor = MISC_DYNAMIC_MINOR,
  .name = NAME,
  .fops = &fops,
  .mode = S_IALLUGO
};
