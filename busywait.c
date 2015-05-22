#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
 
int i = 0; /* i is global, so it is visible to all functions.
                       It's also marked volatile, because it
                       may change in a way which is not predictable by the compiler,
                       here from a different thread. */
 
/* f1 uses a spinlock to wait for i to change from 0. */
static void *f1(void *p)
{
    for (;;) {
        while (i==0) {
            /* do nothing - just keep checking over and over */
            // puts("asdf");
        }
        printf("i's value has changed to %d.\n", i);
    }
    return NULL;
}
 
static void *f2(void *p)
{
    for (;;) {
        sleep(1);   /* sleep for 60 seconds */
        i = 10; 
        // printf("t2 has changed the value of i to %d.\n", i);
        puts("");
        i = 0;
    }
    return NULL;
}

int main()
{
    int rc;
    pthread_t t1, t2;
 
    rc = pthread_create(&t1, NULL, f1, NULL);
    if (rc != 0) {
        fprintf(stderr,"pthread f1 failed\n");
        return EXIT_FAILURE;
    }
 
    rc = pthread_create(&t2, NULL, f2, NULL);
    if (rc != 0) {
        fprintf(stderr,"pthread f2 failed\n");
        return EXIT_FAILURE;
    }
 
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    puts("All pthreads finished.");
    return 0;
}