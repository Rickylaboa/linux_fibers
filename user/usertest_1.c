#include<stdio.h>
#include<pthread.h>
#include"../libs/userfibers.h"

#define NUM_THREADS 2

long nextFiber1;
long nextFiber2;
long nextFiber3;
long nextFiber4;
int ret1=0;
int ret2=0;
int ret3=0;
int ret4=0;


pthread_t threads[NUM_THREADS];


void slave4(void* arg)
{
  printf("{Fiber 4 begin [%ld]}\n",((long int) arg));
  printf("{Fiber 4: end!}\n");
  ret4=1;
  float x = 3.014f;
  float e = 5.104235367353;
  float f = e*x;
  printf("{Fiber 4 using FPU : %f}\n", f);
  switch_to_fiber(nextFiber2);
  f = e*x;
  printf("[Fiber 4: exiting %f!}\n",f);
  exit(0);
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
  float e = 3.104f;
  float x = 103.224552;
  float f = e*x;
  printf("{Fiber 2 using FPU : %f}\n", f);
  printf("{Fiber 2 end ...}\n");
  ret2=1;
  printf("{Fiber 2 waiting}\n");
  while(!all_done());
  printf("{Fiber 2 waked up!}\n");
  switch_to_fiber(nextFiber4);
  printf("[EXITING PROCESS!!]\n");
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
  int conv = convert_thread_to_fiber();
  char* name = ">> Master 2";
  printf("%s: - king fiber begin! -\n",name);
  void (*foo)(void *) = (void*)(slave2);
  nextFiber2 = create_fiber(2<<12,foo,(void*)1);
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


