#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>     
#include<fcntl.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/ioctl.h>    
#include<semaphore.h>  
#include"fiber_constant.h"


static int fd = -1;
static sem_t fiber_sem;
static struct fiber_mem_node *list;

void add_fiber_list(void *stack, int stack_size)
{
  struct fiber_mem_node *new_list = (struct fiber_mem_node *) malloc(sizeof(struct fiber_mem_node));
  if(!new_list){
    printf("Error in malloc() function\n");

    exit(EXIT_FAILURE);
  }
  new_list->stack = stack;
  new_list->stack_size = stack_size; 
  sem_wait(&fiber_sem);
  new_list->next = list;
  list = new_list;
  sem_post(&fiber_sem);
}

struct fiber_mem_node *pop_fiber_list(){

  struct fiber_mem_node *ret;
  sem_wait(&fiber_sem);
  ret = list;
  list = list->next;
  sem_post(&fiber_sem);

  return ret;
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
  long ret; 
  if(fd == -1 ){
    open_device();
  }
  void *stack = aligned_alloc(16,stack_size);
  if(!stack){
    printf("Error in mmap() function\n");

    exit(EXIT_FAILURE);
  }
  struct fiber_info f_info = {
    .stack = (void *)(((unsigned long) stack) + stack_size - 8),   //16 byte alignement of the stack
    .routine = routine,
    .args = args
  };
  add_fiber_list(stack, stack_size);
  
  ret = ioctl(fd, IOCTL_CREATE, (unsigned long)&f_info);

  return ret;
}

long _fls_alloc(){

  if(fd == -1 ){
    open_device();
  }
  long ret = ioctl(fd, IOCTL_FLS_ALLOC, NULL);

  return ret;
}

int _fls_free(long index){

  if(fd == -1 ){
    open_device();
  }
  long ret = ioctl(fd, IOCTL_FLS_FREE, &index);

  return ret;
}

void *_fls_get_value(long index){
  void *lret = 0;
  long ret = 0;
  if(fd == -1 ){
    open_device();
  }

  struct fls_info fls = {
    .index = index,
    .value = NULL 
  };
  ret = ioctl(fd, IOCTL_FLS_GET, &fls);

  return fls.value;
}

int _fls_set_value(long index, void *value){

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

__attribute__((constructor)) void start (void)
{
  sem_init(&fiber_sem,0,1);
}


__attribute__((destructor)) void end (void)
{
  int i = 0;
  close(fd);
  while(list != NULL)
  {
    struct fiber_mem_node *f;
    f = pop_fiber_list();
    free(f->stack);
    free(f);
    i++;
  }
}

