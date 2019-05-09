#include<stdio.h>
#include<pthread.h>
#include"../libs/userfibers.h"

#define NUM_THREADS 200
#define NUM_FIBERS NUM_THREADS*4


long t_fibers[NUM_THREADS];
long c_fibers[NUM_FIBERS];
long mainfiber;
int finished[NUM_FIBERS+NUM_THREADS+1];

pthread_t threads[NUM_THREADS];


void fib2(void* arg)
{
    long int f = (long int) arg;
    printf("[Fiber %ld done!]\n",c_fibers[f]);
    finished[c_fibers[f]]=1;
    while(1);
}

void fib1(void* arg)
{
    long int f = (long int) arg;
    printf("[Fiber %ld done!]\n",c_fibers[f]);
    finished[c_fibers[f]]=1;
    switch_to_fiber(c_fibers[f+1]);
}

int all_done()
{
    int i;
    int ret=1;
    for(i=0; i<NUM_FIBERS+NUM_THREADS+1; i++)
    {
        if(finished[i]==0) 
        {
            ret=0;
            printf("[--- Fiber %d not terminated ---]\n",i);
        }
    }
    printf("\n");
    if(ret)printf("[Main: all terminated!]\n");
    return ret;
}

void* ptfunction(void* start)
{
    long int s = (long int) start;
    int i = s;
    t_fibers[s]=convert_thread_to_fiber();
    printf("[King fiber %ld done]\n",t_fibers[s]);
    finished[t_fibers[s]]=1;
    switch_to_fiber(c_fibers[s*4]);
}


int main()
{
    long i,ret=0,repetitions=0;
    mainfiber = convert_thread_to_fiber();
    printf("[Main fiber begins..]\n");
    for(i=0; i<NUM_FIBERS; i++)
    {
        if((i+1)%4==0)
        {
            c_fibers[i] = create_fiber(2<<12,fib2,((void*)(i)));
        }
        else c_fibers[i] = create_fiber(2<<12,fib1,((void*)(i)));
    }
    printf("\n");
    for(i=0; i<NUM_THREADS; i++){
        ret=pthread_create(&threads[i],NULL,ptfunction,((void*)(i)));
        if(ret<0){
            printf("MAIN failed creation of thread %ld\n",i);
            return 0;
        }
    }
    finished[mainfiber]=1;
    while(!all_done()&&repetitions<2){
        sleep(2);
        repetitions++;
    }
    printf("[Main fiber terminating..]\n");
    exit(0);
}

