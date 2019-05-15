#include <includes/fiber.h>
#include <includes/fls.h>


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
  register_exit_handler();

  if(ret != 0){
    printk(KERN_ERR "%s: Error in misc_register() function\n", NAME);
  }

  return 0;
}

void cleanup_module(void){
  free_all_tables();
  unregister_exit_handler();
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
  struct fls_info *fls;
  long *index;
  long ret;

	switch(cmd)
	{
		case IOCTL_CONVERT: // Userspace requires to convert a thread to fiber
			ret = fiber_convert();
      //printk(KERN_INFO "%s: convert\n",NAME);
      return ret;

		case IOCTL_CREATE: // Userspace requires to create a new fiber
      nfib = (struct fiber_info*) kmalloc(sizeof(struct fiber_info), __GFP_HIGH);
      if(nfib==NULL){
        printk(KERN_ERR "%s: error in kmalloc()\n", NAME);
        return -1;
      }
      copy_from_user(nfib,(struct fiber_info*)ptr,sizeof(struct fiber_info));
			ret = fiber_create((unsigned long) nfib->routine, (unsigned long) nfib->stack, (unsigned long) nfib->args);
      //printk(KERN_INFO "%s: create\n",NAME);
      return ret;
		
    case IOCTL_SWITCH: // Userspace requires to switch from fiber x to fiber y
      index = kmalloc(sizeof(long), __GFP_HIGH);
      if(!index){
        printk(KERN_ERR "%s: Error in kmalloc()\n", NAME);
        return -1;
      }

      copy_from_user(index,(long *)ptr, sizeof(long));
			ret = fiber_switch(*index);
      //printk(KERN_INFO "%s: switch to %ld\n",NAME,(*index));
      return ret;
		
    case IOCTL_FLS_ALLOC: // Userspace requires to switch from fiber x to fiber y
			ret = fls_alloc();
      //printk(KERN_INFO "%s: fls alloc\n",NAME);
      return ret;

    case IOCTL_FLS_FREE: // Userspace requires to switch from fiber x to fiber y
      index = kmalloc(sizeof(long), __GFP_HIGH);
      if(!index){
        //printk(KERN_ERR "%s: Error in kmalloc()\n", NAME);
        return -1;
      }
      copy_from_user(index,(long *)ptr, sizeof(long));
      ret = fls_free(*index);
      //printk(KERN_INFO "%s: fls free\n",NAME);
      return ret;

    case IOCTL_FLS_GET:
      fls = kmalloc(sizeof(struct fls_info),__GFP_HIGH);
      if(!fls){
        printk(KERN_ERR "%s: Error in kmalloc()\n", NAME);
        return -1;
      }
      copy_from_user((void*) fls,(void *)ptr, sizeof(struct fls_info));
      fls->value = fls_get_value(fls->index);
      copy_to_user((void*) ptr, (void*) fls, sizeof(struct fls_info));
      return 0;

    case IOCTL_FLS_SET:
      fls = kmalloc(sizeof(struct fls_info),__GFP_HIGH);
      if(!fls){
        printk(KERN_ERR "%s: Error in kmalloc()\n", NAME);
        return -1;
      }
      copy_from_user(fls,(long *)ptr, sizeof(struct fls_info));
      //printk(KERN_INFO "%s: fls set\n",NAME);
      fls_set_value(fls->index,(void*) fls->value);
      return 0;

		default:
			return 0;		
	}
  
}
