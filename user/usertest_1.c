#include<stdio.h>
#include<pthread.h>
#include<float.h>
#include<sys/types.h>
#include"../libs/userfibers.h"


#define NUM_THREADS 2
#define NUM 10

long nextFiber[4];
int ret[4];

pthread_t threads[NUM_THREADS];


void slave4(void* arg)
{
  printf("{Fiber 4 begin [%ld]}\n", ((long int) arg));
  ret[3] = 1;
  printf("{Fiber 4 before last switching}\n");
  while(1)
  {
    printf("{Fiber 4 ---->}\n");
    sleep(1);
    int ret = -1;
    while(ret<0) ret = switch_to_fiber(nextFiber[1]);
  }
}

int all_done()
{
  int i, value = 1;
  for(i=0; i<4; i++){
    if(!ret[i]) value=0;
  }
  return value;
}

void slave3(void* arg)
{
  printf("{Fiber 3 begin [%ld]}\n",((long int) arg));
  printf("{Fiber 3 end ... }\n");
  long index1 = fls_alloc();
  long index2 = fls_alloc();
  long index3 = fls_alloc();
  long index4;
  printf("{Fiber 3, index values: %ld %ld %ld}\n",index1,index2,index3);

  fls_set_value(index1,(void*)1);
  fls_set_value(index2,(void*)2);
  fls_set_value(index3,(void*)3);
  long v1 = (long) fls_get_value(index1),v2 = (long) fls_get_value(index2), v3 = (long) fls_get_value(index3);
  printf("{Fiber 3, fls values: %ld %ld %ld}\n",v1,v2,v3);
  fls_free(index2);
  index4 = fls_alloc();
  printf("{Fiber 3, index4 is %ld}\n", index4);
  /*fls_free(index1);
  fls_free(index3);
  fls_free(index4);*/


  ret[2] = 1;
  switch_to_fiber(nextFiber[0]);
  while(1)
  {
    printf("{Fiber 3 ---->}\n");
    sleep(1);
    int ret = -1;
    while(ret < 0) ret = switch_to_fiber(nextFiber[1]);  }
}

void slave2(void* arg)
{
  printf("{Fiber 2 begin [%ld]}\n",((long int) arg));
  void (*foo)(void*) = (void*)(slave4);
  nextFiber[3] = create_fiber(2<<12, foo, (void*)3);
  switch_to_fiber(nextFiber[3]);

  long index1 = fls_alloc();
  long index2 = fls_alloc();
  long index3 = fls_alloc();
  long index4;
  printf("{Fiber 2, index values: %ld %ld %ld}\n",index1,index2,index3);

  fls_set_value(index1,(void*)4);
  fls_set_value(index2,(void*)5);
  fls_set_value(index3,(void*)6);
  long v1 = (long) fls_get_value(index1),v2 = (long) fls_get_value(index2), v3 = (long) fls_get_value(index3);
  printf("{Fiber 2, fls values: %ld %ld %ld}\n",v1,v2,v3);
  fls_free(index2);
  index4 = fls_alloc();
  printf("{Fiber 2, index4 is %ld}\n", index4);

  printf("{Fiber 2 end ...}\n");
  ret[1] = 1;
  printf("{Fiber 2 waiting}\n");
  while(!all_done());
  int i = 0;
  while(i < NUM)
  {
    printf("{Fiber 2, iteration: %d/%d}\n", i, NUM);
    printf("{Fiber 2 <----}\n");
    sleep(1);
    switch_to_fiber(nextFiber[i%4]);
    i++;
  }
  printf("{Fiber 2 terminated.}\n");
  exit(0);
}

void slave1(void* arg)
{
  printf("{Fiber 1 begin [%ld]}\n",((long int) arg));
  void (*foo)(void*) = (void*)(slave3);
  nextFiber[2] = create_fiber(2<<12, foo, (void*)3);
  switch_to_fiber(nextFiber[2]);
  
  long index1 = fls_alloc();
  long index2 = fls_alloc();
  long index3 = fls_alloc();
  long index4;
  printf("{Fiber 1, index values: %ld %ld %ld}\n",index1,index2,index3);

  fls_set_value(index1,(void*)7);
  fls_set_value(index2,(void*)8);
  fls_set_value(index3,(void*)9);
  long v1 = (long) fls_get_value(index1),v2 = (long) fls_get_value(index2), v3 = (long) fls_get_value(index3);
  printf("{Fiber 1, fls values: %ld %ld %ld}\n",v1,v2,v3);
  fls_free(index2);
  index4 = fls_alloc();
  printf("{Fiber 1, index4 is %ld}\n", index4);

  printf("{Fiber 1 end ...}\n");
  ret[0] = 1;
  while(1)
  {
    printf("{Master 1 ---->}\n");
    sleep(1);
    int ret = -1;
    while(ret < 0) ret = switch_to_fiber(nextFiber[1]);
  }
}


void* king_one(void* num)
{
  int conv = convert_thread_to_fiber();
  char* name = ">> Master 1";
  printf("%s: - king fiber begin! -\n", name);
  void (*foo)(void *) = (void*)(slave1);
  nextFiber[0] = create_fiber(2<<12, foo, (void*)2);
  switch_to_fiber(nextFiber[0]);
  while(1)
  {
    printf("{Master 1 ---->}\n");
    sleep(1);
    int ret = -1;
    while(ret<0) ret = switch_to_fiber(nextFiber[1]);
  }
}

void* king_two(void* num)
{
  int conv = convert_thread_to_fiber();
  char* name = ">> Master 2";
  printf("%s: - king fiber begin! -\n",name);
  void (*foo)(void *) = (void*)(slave2);
  nextFiber[1] = create_fiber(2<<12, foo, (void*)1);
  switch_to_fiber(nextFiber[1]);
  while(1)
  {
    printf("{Master 2 ---->}\n");
    sleep(1);
    int ret = -1;
    while(ret < 0) ret = switch_to_fiber(nextFiber[1]);
  }
}


int main()
{

  int rc, i;
  rc = pthread_create(&threads[0], NULL, king_one, NULL);
  if(rc > 0)
    {
      printf("Error creating thread\n");
    }	
  int x = 2;
  king_two(&x);
	
}


