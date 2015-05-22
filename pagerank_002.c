#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <xmmintrin.h>
#include <immintrin.h>

#include "pagerank.h"

/* To do list
	> Load bearing.
	Calculate the opitimal load for each thread based on nedges. 
	Iterate through the list and figure out the end location of each thread
	The end is determine when the number of edges are greater 
 */

void displayPageRank (list* plist, double* scores);
double convergeSq(double* curr_scores, double* prev_scores, int npages);

pthread_barrier_t score_barrier;
pthread_barrier_t conv_barrier;

double g_constant;
double* g_sumDiff;
int g_padding;

int g_npages;
int g_nedges;
bool g_hasConverged = false;

typedef struct workerargs
{
	int i;
	int start;
	int end;
	double dampener;
	double constant;
	double * curr_scores;
	double * prev_scores;
	double * outlinks;
	int * inlinks;
	int * page_inlinks;
	int * index_edge;
} workerargs;

void * worker(void * args)
{
	// printf("Yes it's a worker\n");
	workerargs* wargs = (workerargs*) args;

	int start = wargs->start;
	int end = wargs->end;
	double dampener = wargs->dampener;
	double constant = wargs->constant;

	double * curr_scores = wargs->curr_scores;
	double * prev_scores = wargs->prev_scores;
	int * index_edge = wargs->index_edge;

	// int * page_inlinks = wargs->page_inlinks;
	// int * inlinks = wargs->inlinks;
	// double * outlinks = wargs->outlinks;

	// Gains
	int page_inlinks[g_npages];
	memcpy(page_inlinks, wargs->page_inlinks, g_npages * sizeof(int));

	int inlinks[g_nedges];
	memcpy(inlinks, wargs->inlinks, g_nedges * sizeof(int));

	double outlinks[g_nedges];
	memcpy(outlinks, wargs->outlinks, g_nedges * sizeof(double));

	// int * page_inlinks = malloc (g_npages * sizeof(int)); 
	// int * inlinks = malloc (g_nedges * sizeof(int));
	// double * outlinks = malloc (g_nedges * sizeof(double));

	for (;;) /* Calculates the curr_score as many times as it needs */
	{
		int edge;

		if ((start < g_npages) && (g_nedges > 0)) //padding test
			edge = index_edge[start]; /* for edge count */

		/* Swap */
		double* tmp = prev_scores;
		prev_scores = curr_scores;
		curr_scores = tmp;

		double thread_conv = 0.0;

		for (int i = start; i < end; ++i) /* Calculation for each page */
		{
			if (i >= g_npages)
				break; 

			double sum = 0.0;

			for (int j = 0; j < page_inlinks[i]; ++j) /* Calculation for each inlink */
			{
				sum += prev_scores[inlinks[edge]] * outlinks[edge];
				//printf("<%d: %f = %f * %f>\n", i, prev_scores[inlinks[edge]] / outlinks[edge], prev_scores[inlinks[edge]], outlinks[edge]);
				++edge;
			}

			curr_scores[i] = constant + dampener * sum;

			/* Non-SSE calculation of tread_convergence */
			//thread_conv += (curr_scores[i] - prev_scores[i])*(curr_scores[i] - prev_scores[i]);
			//printf("Thread %d: %f\n", wargs->i, thread_conv);
			
			/* SSE Calculation of tread convergence */
			//Go to line 104
		}

		/* AVX version */
		int nblocks = (end - start) * 0.25;
		// printf("<thread: %d> <nblocks: %d> <start: %d> <end: %d>\n", wargs->i, nblocks, start, end);
		for (int i = 0; i < nblocks; i++)
		{
			__m256d* curr_block = (__m256d*) &curr_scores[start]; // Cast it instead. 
			__m256d* prev_block = (__m256d*) &prev_scores[start];

			__m256d m1 = _mm256_sub_pd(curr_block[i], prev_block[i]);
			__m256d m2 = _mm256_mul_pd(m1, m1);

			thread_conv += m2[0] + m2[1] + m2[2] + m2[3];
		}

		// switch ((end - start)%4)
		// {
		// 	case 3:
		// 		thread_conv += (curr_scores[end-3] - prev_scores[end-3])*(curr_scores[end-3] - prev_scores[end-3]);
		// 	case 2:
		// 		thread_conv += (curr_scores[end-2] - prev_scores[end-2])*(curr_scores[end-2] - prev_scores[end-2]);
		// 	case 1:
		// 		thread_conv += (curr_scores[end-1] - prev_scores[end-1])*(curr_scores[end-1] - prev_scores[end-1]);
		// 		break;
		// }


		/* SSE version */
		// int nblocks = (end - start) / 2; /* Fix at 217 */

		//printf("<thread: %d> <nblocks: %d> <start: %d> <end: %d>\n", wargs->i, nblocks, start, end);
		// for (int i = 0; i < nblocks; i ++)
		// {
		// 	/* This gives me alignment issues on test 08, 11, 12 */
		// 	__m128d* curr_block = (__m128d*) &curr_scores[start]; // Cast it instead. 
		// 	__m128d* prev_block = (__m128d*) &prev_scores[start];

		// 	__m128d m1 = _mm_sub_pd(curr_block[i], prev_block[i]);
		// 	__m128d m2 = _mm_mul_pd(m1, m1);

		// 	thread_conv += m2[0] + m2[1];
		// }

		g_sumDiff[wargs->i] = thread_conv; /* Putting the tread_conv so that the main thread can read it. */
		wargs->curr_scores = curr_scores; /* This is just calibrating the curr_scores for the main thread */

		pthread_barrier_wait(&score_barrier); /* Inform that the score is done calculating everything for this run */
		
		/* Put manager function here (if I want one thread to work as a manager as well */
		/* if (wargs->i == 0) //manager thread */

		pthread_barrier_wait(&conv_barrier); /* Wait until converge calculation is complete */

		if (g_hasConverged)
		{
			break;
		}
	}

	return NULL;
}

void displayPageRank (list* plist, double* scores)
{
	node* curr = plist->head;
	int i = 0;

	while(curr != NULL)
	{
		printf("%s %.4f\n", curr->page->name, scores[i]);
		
		curr = curr->next;
		i++;
	}
}

void pagerank(list* plist, int ncores, int npages, int nedges, double dampener)
{
	// Gains
	if (ncores > 1)
		ncores --;

	while ((npages + g_padding) % (ncores * 4) != 0)
	{
		g_padding++;
	}
	
	/***************/
	/* Declaration */
	/***************/
	double constant = (1.0 - dampener) / npages;
	g_npages = npages;
	g_nedges = nedges;

	double * curr_scores;
	double * prev_scores; 
	int * page_inlinks;
	int * inlinks;
	int * index_edge;
	double * outlinks;
	//don't forget gSumDiff

	posix_memalign((void *)&curr_scores, 32, (npages + g_padding) * sizeof(double));
	posix_memalign((void *)&prev_scores, 32, (npages + g_padding) * sizeof(double));
	posix_memalign((void *)&page_inlinks, 32, npages * sizeof(int));
	posix_memalign((void *)&inlinks, 32, nedges * sizeof (int));
	posix_memalign((void *)&index_edge, 32, nedges * sizeof (int));
	posix_memalign((void *)&outlinks, 32, nedges * sizeof (double));
	posix_memalign((void *)&g_sumDiff, 32, ncores * sizeof (double));


	/******************/
	/* Filling arrays */
	/******************/

	node* curr = plist->head;
	unsigned int edge = 0;

	for (int i = 0; i < npages; ++i) /* For each node */
	{
	  // page_list[i] = curr->page; /* page_list array +1 */

		if (curr->page->inlinks != NULL) /* If this page has a inlinks */
		{
			page_inlinks[i] = curr->page->inlinks->length; /* page_inlinks[i] +1 */
			index_edge[i] = edge;

			node* list_node =  curr->page->inlinks->head; /* Setting the first node in the inlinks */

			for (int j = 0; j < curr->page->inlinks->length; ++j)
			{
				inlinks[edge] = list_node->page->index; /* inlinks[edge] +1 */
				outlinks[edge] = 1.0 / list_node->page->noutlinks; /* outlinks[edge] +1*/
				list_node = list_node -> next;
				++edge;
			}
		}

	  curr = curr->next;
	}
	
	/************************/
	/* First iteration (P0) */
	/************************/
	for (int i = 0; i < g_padding; i++)
	{
		curr_scores[npages + i] = 0.0;
		prev_scores[npages + i] = 0.0; //padding
	}

	double p0 = 1.0 / npages;

	for (int i = 0; i < npages; i++)
	{
		curr_scores[i] = p0;
	}
	
	/**************************************/
	/* Second iteration and onwards (P1+) */
	/**************************************/

	pthread_t tids[ncores];
	workerargs wargs[ncores];

	for (int i = 0; i < ncores; i++)
	{
		wargs[i].i = i;
		wargs[i].start = i * ((npages + g_padding) / ncores);
		wargs[i].end = (i + 1) * ((npages + g_padding) / ncores);
		wargs[i].dampener = dampener;
		wargs[i].constant = constant;
		wargs[i].curr_scores = curr_scores;
		wargs[i].prev_scores = prev_scores;
		wargs[i].outlinks = outlinks;
		wargs[i].inlinks = inlinks;
		wargs[i].page_inlinks = page_inlinks;
		wargs[i].index_edge = index_edge;
	}

	wargs[ncores-1].end = npages + g_padding;
	
	pthread_barrier_init(&score_barrier, NULL, ncores+1);
	pthread_barrier_init(&conv_barrier, NULL, ncores+1);

	for (int i = 0; i < ncores; i++)
		pthread_create(&tids[i], NULL, worker, &wargs[i]);

	for (;;)
	{
		pthread_barrier_wait(&score_barrier);

		double conv = 0.0;

		for (int i = 0; i < ncores; i++)
		{
			conv += g_sumDiff[i];
		}
		
		if (conv < EPSILON*EPSILON)
		{
			g_hasConverged = true;
			break;
		}

		pthread_barrier_wait(&conv_barrier);
	}

	pthread_barrier_wait(&conv_barrier);



	curr_scores = wargs[0].curr_scores; 
	/* Because the main thread doesn't swap - the final result it prints could have be really prev_score */
	/* This way just calibrates so that curr_score in main is really the curr_score */
  
	displayPageRank(plist, curr_scores);
	for (int i = 0; i < ncores; i++)
		pthread_join(tids[i], NULL);

	pthread_barrier_destroy(&score_barrier);
	pthread_barrier_destroy(&conv_barrier);

	/******************/
	/* Freeing memory */
	/******************/
	/*
	free(curr_scores);
	free(prev_scores);
	// free(page_list);
	free(index_edge);
	free(page_inlinks);
	free(outlinks);
	free(inlinks);
	free(g_sumDiff);
	*/
}

/*
######################################
### DO NOT MODIFY BELOW THIS POINT ###
######################################
*/

int main(void) {

	/*
	######################################################
	### DO NOT MODIFY THE MAIN FUNCTION OR HEADER FILE ###
	######################################################
	*/

	list* plist = NULL;

	double dampener;
	int ncores, npages, nedges;

	/* read the input then populate settings and the list of pages */
	read_input(&plist, &ncores, &npages, &nedges, &dampener);

	/* run pagerank and output the results */
	pagerank(plist, ncores, npages, nedges, dampener);

	/* clean up the memory used by the list of pages */
	page_list_destroy(plist);

	return 0;
}
