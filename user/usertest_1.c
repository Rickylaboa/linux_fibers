#include<stdio.h>
#include<pthread.h>
#include"../libs/userfibers.h"

#define NUM_THREADS 2

int nextFiber1;
int nextFiber2;
int nextFiber3;
int nextFiber4;
int ret1=0;
int ret2=0;
int ret3=0;
int ret4=0;


pthread_t threads[NUM_THREADS];


void slave4(void* arg)
{
  printf("{Fiber 4 begin [%ld]}\n",((long int) arg));
  printf("{Fiber 4: switching from %d to %d}\n",nextFiber4,nextFiber2);
  printf("{Fiber 4: end!}\n");
  ret4=1;
  switch_to_fiber(nextFiber2);
  printf("[Fiber 4: UNREACHABLE!}\n");
}

int all_done()
{
  return ret1&&ret2&&ret3&&ret4;
}

void slave3(void* arg)
{
  printf("{Fiber 3 begin [%ld]}\n",((long int) arg));
  printf("{Fiber 3 end ... }\n");
  ret3=1;
  switch_to_fiber(nextFiber1);
}

void slave2(void* arg)
{
  printf("{Fiber 2 begin [%ld]}\n",((long int) arg));
  void (*foo)(void*)=(void*)(slave4);
  nextFiber4= create_fiber(2<<12,foo,(void*)3);
  switch_to_fiber(nextFiber4);
  printf("{Fiber 2 end ...}\n");
  ret2=1;
  while(!all_done())
    {
      
      printf("{Fiber 2 waiting}\n");
    }
  printf("[EXITING PROCESS!!]\n");
  exit(0);
}

void slave1(void* arg)
{
  printf("{Fiber 1 begin [%ld]}\n",((long int) arg));
  void (*foo)(void*)=(void*)(slave3);
  nextFiber3= create_fiber(2<<12,foo,(void*)3);
  switch_to_fiber(nextFiber3);
  printf("{Fiber 1 end ...}\n");
  ret1=1;
  while(1);
}


void* king_one(void* num)
{
  int conv= convert_thread_to_fiber();
  char* name=">> Master 1";
  printf("%s: - king fiber begin! -\n",name);
  void (*foo)(void *)=(void*)(slave1);
  nextFiber1= create_fiber(2<<12,foo,(void*)2);
  switch_to_fiber(nextFiber1);
}

void* king_two(void* num)
{
  int conv= convert_thread_to_fiber();
  char* name=">> Master 2";
  printf("%s: - king fiber begin! -\n",name);
  void (*foo)(void *)=(void*)(slave2);
  nextFiber2= create_fiber(2<<12,foo,(void*)1);
  switch_to_fiber(nextFiber2);
}


int main()
{

  int rc, i;
  rc = pthread_create(&threads[0],NULL,king_one,NULL);
  if(rc>0)
    {
      printf("Error creating thread\n");
    }	
  int x=2;
  king_two(&x);
	
}


