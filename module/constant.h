#ifndef CONSTANT_H // Fabio Marra's copyright
#define CONSTANT_H

#define ACTIVE_FIBER 1
#define INACTIVE_FIBER 0

#define NAME "fibers"    //device name
#define F_MAJOR 10     //global major number, set to 10 because this is a misc device


#define IOCTL_CONVERT _IO(F_MAJOR, 0)
#define IOCTL_SWITCH _IOW(F_MAJOR, 1, long)
#define IOCTL_CREATE _IOW(F_MAJOR, 2, void *)
#define IOCTL_FLS_ALLOC _IOW(F_MAJOR, 3, void *)
#define IOCTL_FLS_FREE _IOW(F_MAJOR, 4, void *)
#define IOCTL_FLS_GET _IOW(F_MAJOR, 5, void *)
#define IOCTL_FLS_SET _IOW(F_MAJOR, 6, void *)


struct fiber_info { // data to copy from user in order to create a new fiber

  void *stack;
  void (*routine)(void *);
  void *args;
};

#endif