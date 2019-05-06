#include<linux/ioctl.h>

#define NAME "fibers"    //device name
#define MAJOR 10     //global major number, set to 10 because this is a misc device


//ioctl macros

#define IOCTL_CONVERT _IO(MAJOR, 0)
#define IOCTL_SWITCH _IOW(MAJOR, 1, long)
#define IOCTL_CREATE _IOW(MAJOR, 2, void *)

// to do fls ioctl macros...


struct fiber_info {        //useful info to create a fiber

  void *stack;
  void (*routine) (void *);
  void *args;
};


// fls to do...


