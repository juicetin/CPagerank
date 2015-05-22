#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

pthread_t threads[5];
pthread_barrier_t my_barrier;

typedef struct {
	int * array, start, end, sum;
} wargs;

void * worker (void *arg)
{
	wargs sum[5];
	for (int i = sum->start; i < sum->end; i++)
	{
		sum->sum += sum->array[i];
	}
	return NULL;
}

int main (void) {
	pthread_barrier_init(&my_barrier, NULL, 5);

	int i, j;
	int test[20];
	for (i = 0; i < 20; i++)
		test[i] = i;

	
	pthread_t threads[5];
	wargs wargs[5];

	int splitter = 20/5;

	for (int i = 0; i < 5; i++)
	{
		wargs[i].array = test;
		wargs[i].start = i * splitter;
		wargs[i].end = (i+1) * splitter;
	}
	wargs[5-1].end = 20;

	for (i = 0; i < 5; i++)
		pthread_create(&threads[i], NULL, worker, &wargs[i]);

	for (i = 0; i < 5; i++)
		pthread_join(threads[i], NULL);

	j = 0;
	for (i = 0; i < 5; i++)
		j += wargs[i].sum;

	printf("AND THE SUM IS ======> %d\n", j);

	pthread_exit(NULL);
	return 0;
}