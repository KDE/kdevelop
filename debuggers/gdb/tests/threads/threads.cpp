/* This is a test program for KDevelop GDB debugger support.

   There are two worker threads, they are programmed to call
   the 'foo' function in strictly interleaved fashion.
*/

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

int schedule[] = {1, 2};
int schedule_size = sizeof(schedule)/sizeof(schedule[0]);
int index = 0;
int exit = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

void foo(int thread, int i)
{
    printf ("hi there, from thread %d on iteration %d\n", thread, i);
}

void runner(int id)
{
    for(int i = 0; i < 1000000 && !exit; ++i)
    {
        pthread_mutex_lock(&mutex);

        while (schedule[index] != id) {
            pthread_cond_wait(&condition, &mutex);
        }

        foo(id, i);
        
        ++index;
        if (index >= schedule_size)
            index = 0;
               
        pthread_cond_broadcast(&condition);
        pthread_mutex_unlock(&mutex);

        sleep(1);
    }
}

void* thread(void* p)
{
    runner((int)p);
    return NULL;
}

int main()
{
    pthread_t p1, p2;
    
    pthread_create(&p1, 0, &thread, (void*)1);
    pthread_create(&p2, 0, &thread, (void*)2);    
    
    pthread_join(p1, 0);
    pthread_join(p2, 0);
    
    return 0;
}
