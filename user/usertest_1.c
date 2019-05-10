#include<stdio.h>
#include<pthread.h>
#include"../libs/userfibers.h"

#define NUM_THREADS 2

long nextFiber[4];
int ret[4];


pthread_t threads[NUM_THREADS];

float op(float f1, float f2)
{
  return f1*f2;
}

void slave4(void* arg)
{
  register float x;
  printf("{Fiber 4 begin [%ld]}\n",((long int) arg));
  printf("{Fiber 4: end!}\n");
  ret[3]=1;
  //x = op(1.234234534563,2.23556457745);
  //printf("{Fiber 4 using FPU : %f}\n", x);
  switch_to_fiber(nextFiber[1]);
  printf("{Fiber 4 before exiting}\n");

  exit(0);
}

int all_done()
{
  printf("{%d %d %d %d}\n",ret[0],ret[1],ret[2],ret[3]);
  return ret[0]&&ret[1]&&ret[2]&&ret[3];
}

void slave3(void* arg)
{
  printf("{Fiber 3 begin [%ld]}\n",((long int) arg));
  printf("{Fiber 3 end ... }\n");
  ret[2]=1;
  switch_to_fiber(nextFiber[0]);
}

void slave2(void* arg)
{
  printf("{Fiber 2 begin [%ld]}\n",((long int) arg));
  void (*foo)(void*)=(void*)(slave4);
  nextFiber[3]= create_fiber(2<<12,foo,(void*)3);
  switch_to_fiber(nextFiber[3]);
  //register float f = op(21.2342345235,24.234256);
  //printf("{Fiber 2 using FPU : %f}\n", f);
  printf("{Fiber 2 end ...}\n");
  ret[1]=1;
  printf("{Fiber 2 waiting}\n");
  while(!all_done());
  printf("{Fiber 2 waked up!}\n");
  switch_to_fiber(nextFiber[3]);
  printf("[EXITING PROCESS!!]\n");
}

void slave1(void* arg)
{
  printf("{Fiber 1 begin [%ld]}\n",((long int) arg));
  void (*foo)(void*)=(void*)(slave3);
  nextFiber[2]= create_fiber(2<<12,foo,(void*)3);
  switch_to_fiber(nextFiber[2]);
  printf("{Fiber 1 end ...}\n");
  ret[0]=1;
  while(1);
}


void* king_one(void* num)
{
  int conv= convert_thread_to_fiber();
  char* name=">> Master 1";
  printf("%s: - king fiber begin! -\n",name);
  void (*foo)(void *)=(void*)(slave1);
  nextFiber[0]= create_fiber(2<<12,foo,(void*)2);
  switch_to_fiber(nextFiber[0]);
}

void* king_two(void* num)
{
  int conv = convert_thread_to_fiber();
  char* name = ">> Master 2";
  printf("%s: - king fiber begin! -\n",name);
  void (*foo)(void *) = (void*)(slave2);
  nextFiber[1] = create_fiber(2<<12,foo,(void*)1);
  switch_to_fiber(nextFiber[1]);
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


