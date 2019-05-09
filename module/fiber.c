#include "fiber.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Fabio Marra <fab92mar@gmail.com> & Riccardo Valentini <valentiniriccardo05@gmail.com>");
MODULE_DESCRIPTION("Kernel module implementing Windows fibers on Linux");
 

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

int init_module(void){

  int ret = misc_register(&mdev);
  init_hashtables();  
  if(ret != 0){
    printk(KERN_ERR "%s: Error in misc_register() function\n", NAME);
  }

  return 0;
}

void cleanup_module(void){

  free_all_tables();
  misc_deregister(&mdev);
}


static int hit_open(struct inode *inode, struct file *file){    // to do...

  return 0;
}


static int hit_release(struct inode *inode, struct file *file){    // to do...

  return 0;
}


static long hit_ioctl(struct file *filp, unsigned int cmd, unsigned long __user ptr){

  struct fiber_info *nfib;
  long *index;

	switch(cmd)
	{
		case IOCTL_CONVERT: // Userspace requires to convert a thread to fiber
			printk(KERN_INFO "%s: CONVERT\n", NAME);
			return fiber_convert();

		case IOCTL_CREATE: // Userspace requires to create a new fiber
			printk(KERN_INFO "%s: CREATE\n", NAME);
      nfib = (struct fiber_info*) kmalloc(sizeof(struct fiber_info), __GFP_HIGH);
      if(nfib==NULL){
        printk(KERN_ERR "%s: error in kmalloc()\n",NAME);
        return -1;
      }
      copy_from_user(nfib,(struct fiber_info*)ptr,sizeof(struct fiber_info));
			return fiber_create((unsigned long) nfib->routine, (unsigned long) nfib->stack, (unsigned long) nfib->args);

		case IOCTL_SWITCH: // Userspace requires to switch from fiber x to fiber y
			printk(KERN_INFO "%s: SWITCH\n", NAME);
      index = kmalloc(sizeof(long), __GFP_HIGH);
      if(!index){
        printk(KERN_ERR "%s: Error in kmalloc()\n", NAME);
        return -1;
      }

      copy_from_user(index,(long *)ptr, sizeof(long));
			return fiber_switch(*index);

		default:
			return 0;		
	}
  
}
