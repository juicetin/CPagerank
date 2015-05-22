#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>

/* create a barrier */
pthread_barrier_t barrier;

/* normal thread */
void* robot_worker(void *arg)
{
    
    for(;;)
    {
        char* msg = (char *)arg;
        puts(msg);
        pthread_barrier_wait(&barrier);
        /* advancing the conveyor belt happens between these two barriers */
        pthread_barrier_wait(&barrier);
    }

    return NULL;
}

/* manager */
void* robot_worker_manager(void* arg)
{
    for(;;)
    {
        /* wait for all threads to print message */
        pthread_barrier_wait(&barrier);
        /* advance conveyor belt */
        puts("Advancing conveyor belt");
        usleep(750000);
        puts("Next work item read.\n");
        /* force threads to wait for the conveyor belt to advance */
        pthread_barrier_wait(&barrier);
    }

    return NULL;
}




int main(void)
{
    char *wkrs[] = {
        "cutting",
        "welding",
        "painting"
    };

    int nrobots = sizeof(wkrs) / sizeof(wkrs[0]);
    pthread_t tids[nrobots];
    pthread_t manager;
    pthread_barrier_init(&barrier, NULL, nrobots+1);

    //for(;;)
    //{
        /* create manager */
        pthread_create(&manager, NULL, robot_worker_manager,NULL);

        for(int i = 0; i < nrobots; i++)
        {
            pthread_create(&tids[i],NULL,robot_worker,wkrs[i]);
        }

        for(int i = 0; i < nrobots; i++)
        {
            pthread_join(tids[i], NULL);
        }
        /*
        puts("Advancing conveyor belt...");
        usleep(750000);
        puts("Next work item read.");
        */
    //}

    return 0;
}

