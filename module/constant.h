#define ACTIVE_FIBER 1
#define INACTIVE_FIBER 0

#define NAME "fibers"    //device name
#define MAJOR 10     //global major number, set to 10 because this is a misc device


#define IOCTL_CONVERT _IO(MAJOR, 0)
#define IOCTL_SWITCH _IOW(MAJOR, 1, long)
#define IOCTL_CREATE _IOW(MAJOR, 2, void *)

struct fiber_info { // data to copy from user in order to create a new fiber

  void *stack;
  void (*routine)(void *);
  void *args;
};