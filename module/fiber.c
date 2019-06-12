#include<includes/fiber.h>
#include<includes/fls.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Fabio Marra <fab92mar@gmail.com> && Riccardo Valentini <valentiniriccardo05@gmail.com>");
MODULE_DESCRIPTION("Kernel module implementing Windows fibers on Linux");


int init_module(void){

  int ret = misc_register(&mdev);
  if(ret != 0){
    printk(KERN_ERR "%s: Error in misc_register() function\n", NAME);

    return -1;
  }

  init_hashtables(); 
  register_exit_handler();
  proc_init();

  return 0;
}

void cleanup_module(void){

  proc_end();
  unregister_exit_handler();
  misc_deregister(&mdev);
}


static int hit_open(struct inode *inode, struct file *file){    

  return 0;
}


static int hit_release(struct inode *inode, struct file *file){   

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
      nfib = (struct fiber_info *) kzalloc(sizeof(struct fiber_info), GFP_KERNEL);
      if(unlikely(!nfib)){
        printk(KERN_ERR "%s: error in kzalloc()\n", NAME);

        return -1;
      }

      copy_from_user(nfib, (struct fiber_info *)ptr, sizeof(struct fiber_info));
			ret = fiber_create((unsigned long) nfib->routine, (unsigned long) nfib->stack, (unsigned long) nfib->args);
      //printk(KERN_INFO "%s: create\n",NAME);
      kfree(nfib);

      return ret;
		
    case IOCTL_SWITCH: // Userspace requires to switch from fiber x to fiber y
      index = kzalloc(sizeof(long), GFP_KERNEL);
      if(unlikely(!index)){
        printk(KERN_ERR "%s: Error in kzalloc()\n", NAME);

        return -1;
      }

      copy_from_user(index, (long *)ptr, sizeof(long));
			ret = fiber_switch(*index);
      //printk(KERN_INFO "%s: switch to %ld\n",NAME,(*index));
      kfree(index);

      return ret;
		
    case IOCTL_FLS_ALLOC: // Userspace requires to switch from fiber x to fiber y
			ret = fls_alloc();
      //printk(KERN_INFO "%s: fls alloc\n",NAME);
      return ret;

    case IOCTL_FLS_FREE: // Userspace requires to switch from fiber x to fiber y
      index = kzalloc(sizeof(long), GFP_KERNEL);
      if(unlikely(!index)){
        //printk(KERN_ERR "%s: Error in kzalloc()\n", NAME);
        return -1;
      }
      copy_from_user(index, (long *)ptr, sizeof(long));
      ret = fls_free(*index);
      //printk(KERN_INFO "%s: fls free\n",NAME);
      kfree(index);

      return ret;

    case IOCTL_FLS_GET:
      fls = kzalloc(sizeof(struct fls_info), GFP_KERNEL);
      if(unlikely(!fls)){
        printk(KERN_ERR "%s: Error in kzalloc()\n", NAME);

        return -1;
      }
      copy_from_user((void *) fls,(void *)ptr, sizeof(struct fls_info));
      fls->value = fls_get_value(fls->index);
      copy_to_user((void *) ptr, (void *) fls, sizeof(struct fls_info));
      kfree(fls);

      return 0;

    case IOCTL_FLS_SET:
      fls = kzalloc(sizeof(struct fls_info), GFP_KERNEL);
      if(unlikely(!fls)){
        printk(KERN_ERR "%s: Error in kzalloc()\n", NAME);

        return -1;
      }
      copy_from_user(fls, (long *)ptr, sizeof(struct fls_info));
      //printk(KERN_INFO "%s: fls set\n",NAME);
      fls_set_value(fls->index,(void*) fls->value);
      kfree(fls);

      return 0;

		default:

			return 0;		
	}
}