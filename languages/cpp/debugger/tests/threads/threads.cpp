
#include <pthread.h>

void runner(int i)
{
    for(int i = 0; i < 1000000;)
        ++i;
}

void* thread(void* p)
{
    runner((int)p);
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