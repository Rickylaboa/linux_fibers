#include<linux/ioctl.h>

#define NAME "fibers"    //device name
#define MAJOR 10     //global major number, set to 10 because this is a misc device


//ioctl macros

#define IOCTL_CONVERT _IO(MAJOR, 0)
#define IOCTL_SWITCH _IOW(MAJOR, 1, long)
#define IOCTL_CREATE _IOW(MAJOR, 2, void *)
#define IOCTL_FLS_ALLOC _IOW(F_MAJOR, 3, void *)
#define IOCTL_FLS_FREE _IOW(F_MAJOR, 4, void *)
#define IOCTL_FLS_GET _IOW(F_MAJOR, 5, void *)
#define IOCTL_FLS_SET _IOW(F_MAJOR, 6, void *)


struct fiber_info {        //useful info to create a fiber

  void **stack;
  void (*routine) (void *);
  void *args;
};


// fls to do...


