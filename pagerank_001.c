#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "pagerank.h"


static inline void swap (double** curr_scores, double** prev_scores)
{
    double* tmp = *prev_scores;
    *prev_scores = *curr_scores;
    *curr_scores = tmp;
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

double convergeSq(double* curr_scores, double* prev_scores, int npages)
{
    double sum = 0.0;
    // double curr;
    // double prev;

    for (int i = 0; i < npages; ++i)
    {
        double curr = curr_scores[i];
        double prev = prev_scores[i];
        sum += curr*curr - 2*curr*prev + prev*prev;
    	//sum += curr_scores[i]*curr_scores[i] - 2*curr_scores[i]*prev_scores[i] + prev_scores[i]*prev_scores[i];
    }

    return sum;
}

void pagerank(list* plist, int ncores, int npages, int nedges, double dampener)
{
    /***************/
    /* Declaration */
    /***************/
    const double constant = (1.0 - dampener) / npages;

    /* Stores the scores for this round */
    double * curr_scores =  calloc (npages, sizeof(double)); 

	/* Stores the score for last round - need for calculating converce */
    double * prev_scores = calloc (npages, sizeof(double)); 
    
    /* Stores a pointer to each page */
    // page ** page_list = (page **) calloc (npages, sizeof(page*));

    /* Number of outlinks per page. Usage: page_outlinks[i] where i is the page index*/
    int * page_inlinks = calloc (npages, sizeof(int)); //determines how many loops
    
    /* inlinks[edge] = index */
    int * inlinks = calloc (nedges, sizeof(int)); //the index for the numerator prev_score[inlinks[edge]];

    /* outlinks[edge] = noutlinks */
    double * outlinks = calloc (nedges, sizeof(double)); //the numerator

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

  			node* list_node =  curr->page->inlinks->head; /* Setting the first node in the inlinks */

  			for (int j = 0; j < curr->page->inlinks->length; ++j)
	  		{
	  			inlinks[edge] = list_node->page->index; /* inlinks[edge] +1 */
	  			outlinks[edge] = 1.0 / list_node->page->noutlinks; /* outlinks[edge] +1*/
	  			// printf("<EDGE %d for %s>", edge, page_list[i]->name);
	  			// printf(" <LOOPS: %d>", page_inlinks[i]);
	  			// printf(" <IN: %s, %d>", list_node->page->name, list_node->page->index);
	  			// printf(" <OUTLINKS: %d for %s>\n", list_node->page->noutlinks, list_node->page->name);
	  			list_node = list_node -> next;
	  			++edge;
	  		}
  		}

  		curr = curr->next;
  	}

  	// for (int i = 0; i < npages; i++)
  	// {
  	// 	printf("<%d, %s>\n", i, page_list[i]->name);
  	// }

  	// for (edge = 0; edge < nedges; edge++)
  	// {
  	// 	printf("<IN: %s %d>\n", page_list[inlinks[edge]]->name, inlinks[edge]);
  	// }
  	
  	/************************/
  	/* First iteration (P0) */
  	/************************/

  	double p0 = 1.0 / npages;

    for (int i = 0; i < npages; i++)
    {
        curr_scores[i] = p0;
    }

    //displayPageRank(plist, curr_scores);
    /**************************************/
  	/* Second iteration and onwards (P1+) */
  	/**************************************/
    do
    {
    	edge = 0; /* for edge count */

    	/* Swap */
    	double* tmp = prev_scores;
	    prev_scores = curr_scores;
	    curr_scores = tmp;

	    // displayPageRank(plist, curr_scores);

	    for (int i = 0; i < npages; ++i)
	    {
	    	double sum = 0.0;

	    	for (int j = 0; j < page_inlinks[i]; ++j)
	    	{
	    		sum += prev_scores[inlinks[edge]] * outlinks[edge];
	    		// printf("<%s: %f = %f / %d>\n", page_list[i], prev_scores[inlinks[edge]] / outlinks[edge], prev_scores[inlinks[edge]], outlinks[edge]);
	    		++edge;
	    	}

	    	curr_scores[i] = constant + dampener * sum;
	    }


    } while (convergeSq(curr_scores, prev_scores, npages) > EPSILON*EPSILON);
	
	displayPageRank(plist, curr_scores);


    /******************/
  	/* Freeing memory */
  	/******************/
    free(curr_scores);
    free(prev_scores);
    // free(page_list);
    free(page_inlinks);
    free(outlinks);
    free(inlinks);
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
