#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>     
#include<fcntl.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/ioctl.h>      
#include"constant.h"


static int fd = -1;

long system_architecture(void) {
    long wordBits = sysconf(_SC_WORD_BIT);
    if (wordBits == -1){
      return -1;
    }
    return wordBits;
}

void open_device(){ 

  fd = open("/dev/fibers", 0);
  if(fd < 0){
    printf("Error in opening %s device\n", NAME);
    exit(EXIT_FAILURE);
  }
}

long convert_thread_to_fiber(){

  if(fd == -1 ){
    open_device();
  }
  long ret = ioctl(fd, IOCTL_CONVERT, 0);

  return ret;
}


long switch_to_fiber(long index){

  if(fd == -1 ){
    open_device();
  }
  long ret = ioctl(fd, IOCTL_SWITCH, &index);

  return ret;
}

long create_fiber(size_t stack_size,void (*routine)(void *), void *args){

  if(fd == -1 ){
    open_device();
  }
  void *stack = mmap(NULL, stack_size, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANON, -1, 0);
  if(!stack){
    printf("Error in mmap() function\n");
    exit(EXIT_FAILURE);
  }

  struct fiber_info f_info = {

    .stack = (void **)(((unsigned long) stack) + stack_size - 8),   //16 byte alignement of the stack
    .routine = routine,
    .args = args
  };
  
  long ret = ioctl(fd, IOCTL_CREATE, (unsigned long)&f_info);
  return ret;
}

long fls_alloc(){
  if(fd == -1 ){
    open_device();
  }
  long ret = ioctl(fd, IOCTL_FLS_ALLOC, NULL);
  return ret;
}

int fls_free(long index){
  if(fd == -1 ){
    open_device();
  }
  long ret = ioctl(fd, IOCTL_FLS_FREE, &index);
  return ret;
}

void* fls_get_value(long index){
  void* lret = 0;
  if(fd == -1 ){
    open_device();
  }
  lret = (void*) (long) ioctl(fd, IOCTL_FLS_GET, &index);
  return lret;
}

int fls_set_value(long index,void* value){
  if(fd == -1 ){
    open_device();
  }

  struct fls_info fls = {
    .index = index,
    .value = value
  };
  int ret = ioctl(fd, IOCTL_FLS_SET, &fls);
  return ret;
}

