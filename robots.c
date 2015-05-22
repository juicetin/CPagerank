#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>

pthread_barrier_t wk_barrier;
pthread_barrier_t man_barrier;

void *robot_worker(void *arg)
{
	for (;;)
	{
		char *msg = (char *)arg;
		puts(msg);

		pthread_barrier_wait(&wk_barrier); //Only continue when everyone is done
		pthread_barrier_wait(&man_barrier); //continue next iteration when the converyer belt is done
	}

	return NULL;
}

void *manager_worker(void *arg)
{
	for (;;)
	{		
		pthread_barrier_wait(&wk_barrier); //wait for three works than advance.

		puts("Advancing conveyor belt...");
		usleep(750000);
		puts("Next work item ready.");

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
	
	pthread_barrier_init(&wk_barrier, NULL, 4);
	pthread_barrier_init(&man_barrier, NULL, 4);


	pthread_create(&tids[0], NULL, robot_worker, wkrs[0]);
	pthread_create(&tids[1], NULL, robot_worker, wkrs[1]);
	pthread_create(&tids[2], NULL, robot_worker, wkrs[2]);
	pthread_create(&tids[3], NULL, manager_worker, NULL);

	for (int i = 0; i < 4; i++)
		pthread_join(&tids[i], NULL);

	return 0;
} 
