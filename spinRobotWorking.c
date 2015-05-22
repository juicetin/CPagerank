#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include <pthread.h>

//pthread_barrier_t wk_barrier;
pthread_barrier_t man_barrier;
//pthread_spinlock_t wk_barrier;

#define ROUNDS 500000

typedef struct workerargs
{
	char * job;
	int index;
} workerargs;

double b[3] = {0};
bool proceed = false;

void *robot_worker(void *arg)
{
	workerargs* warg = (workerargs*) arg;

	char *msg = warg->job;
	int index = warg->index; 
	
	for (int i = 0; i < ROUNDS; i++)
	{
		b[index] = 1;

		while (!proceed);

		// puts(msg);
		
		b[index] = 0;
		pthread_barrier_wait(&man_barrier); //continue next iteration when the converyer belt is done
	}

	return NULL;
}

void *manager_worker(void *arg)
{
	for (int i = 0; i < ROUNDS; i++)
	{
		// pthread_barrier_wait(&wk_barrier); //wait for three works than advance.

		while (b[0] + b[1] + b[2] != 3);
		proceed = true;

		while (b[0] + b[1] + b[2] != 0);
		proceed = false;


		// puts("Advancing conveyor belt...");
		// usleep(75000);
		// puts("Next work item ready.");

		pthread_barrier_wait(&man_barrier); //gives the green after moving the belt
	}

	return NULL;
}


int main(void) {
	// The jobs for each robot arm to execute. We'll launch a worker
	// thread per job.
	char *wkrs[] = {
		"cutting",
		"welding",
		"painting"
	};

	pthread_t tids[4];
	workerargs wargs[3];

	//pthread_barrier_init(&wk_barrier, NULL, 4);
	pthread_barrier_init(&man_barrier, NULL, 4);

	for (int i = 0; i < 3; i++)
	{
		wargs[i].job = wkrs[i];
		wargs[i].index = i;
	}

	pthread_create(&tids[0], NULL, robot_worker, &wargs[0]);
	pthread_create(&tids[1], NULL, robot_worker, &wargs[1]);
	pthread_create(&tids[2], NULL, robot_worker, &wargs[2]);
	pthread_create(&tids[3], NULL, manager_worker, NULL);

	for (int i = 0; i < 4; i++)
		pthread_join(tids[i], NULL);

	return 0;
} 
