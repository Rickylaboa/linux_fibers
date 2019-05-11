#include<stdio.h>
#include<pthread.h>
#include"../libs/userfibers.h"

/*  Main thread converts to a fiber, then creates a number of fibers depending on NUM_THREADS. 
    It will spawn later NUM_THREADS threads, each of one converting to fiber.
    All threads, except the main one, will switch to a fiber beginning a chain of 3 consecutive switches. 
    Each thread will write on an array to notify nothing more to do, in such a way the main can exit when all the job
    is done!
    
    Notice that NUM_THREADS can be at most 409, because in this way the number of fibers created by this process will be
    409 + 4x409 + 1 = 410 + 1636 = 2046 < 2048 (maximum number of fibers). 
    Adding just one thread will cause some fibers impossible to create, causing kernel errors ;-) */
#define NUM_THREADS 100 
#define NUM_FIBERS NUM_THREADS*4


long t_fibers[NUM_THREADS];
long c_fibers[NUM_FIBERS];
long mainfiber;
int finished[NUM_FIBERS+NUM_THREADS+1];

pthread_t threads[NUM_THREADS];

long get_random_fiber()
{
    long n = (long) rand() % NUM_FIBERS + 1;
    return n;
}


/*  Function to print a fiber had worked,
    switching to the next fiber.*/
__attribute__((fastcall)) void fib1(void* arg) 
{
    int ret = -1;
    long f = (long) arg;
    printf("[Fiber %ld done!]\n",c_fibers[f]);
    finished[c_fibers[f]]=1;
    while(1){
        long rf=get_random_fiber();
        while(finished[rf]) rf = (rf + 1)%(NUM_FIBERS+1);
        printf("[Fiber %ld selected %ld]\n",c_fibers[f],c_fibers[rf]);
        ret=switch_to_fiber(rf);
        if(ret<0) continue;
    }
}

/*  Function checking the shared array, guessing
    if all fibers have finished.*/
int all_done()
{
    int i;
    int ret=1;
    for(i=0; i<NUM_FIBERS+NUM_THREADS+1; i++){
        if(finished[i]==0){
            ret=0;
        }
    }
    if(ret)printf("[Main: all terminated!]\n");
    return ret;
}


/*  Thread function, issues convert to fiber, print out
    that had worked and then switches to a proper fiber.*/
void* ptfunction(void* start)
{
    long s = (long) start;
    int i = s;
    t_fibers[s]=convert_thread_to_fiber();
    printf("[King fiber %ld done]\n",t_fibers[s]);
    finished[t_fibers[s]]=1;
    int ret=-1;
    while(ret<0) ret = switch_to_fiber(c_fibers[s]);
}

/*  Main thread converts to fiber, creates NUM_FIBER number of fibers, NUM_THREAD number
    of threads. Notice that 1 fiber out of 4 is created with function fib2 (the
    one pausing forever at the end, while the other 3 fibers are created with the
    function fib1, switching to the next fiber at the end. All threads are instead created
    with the function ptfunction. 
    
    In the second part, the main thread notify it has finished, waiting for the all fibers. 
    When all work is done, finally, main thread exits with success! */
int main()
{
    int num_fibers, num_threads;
    long i,ret=0;
    printf("System is %ld bits\n",system_architecture());
    mainfiber = convert_thread_to_fiber();
    printf("[Main fiber begins..]\n");
    for(i=0; i<NUM_FIBERS; i++)
    {
        void* funct = (void*) fib1;
        c_fibers[i] = create_fiber(2<<12,funct,((void*)(i)));
    }
    for(i=0; i<NUM_THREADS; i++){
        ret=pthread_create(&threads[i],NULL,ptfunction,((void*)(i)));
        if(ret<0){
            printf("MAIN failed creation of thread %ld\n",i);
            return 0;
        }
    }
    finished[mainfiber]=1;
    while(!all_done());
    printf("[Main fiber terminating..]\n");
    exit(0);
}

