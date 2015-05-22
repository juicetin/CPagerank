/*I recommend using Sublime Text to read this file.
The task was a C implementation of the original Google Pagerank algorithm.
I placed 4th out of a cohort of 300 in this competitive ranked assignment.*/

#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "pagerank.h"
#include <xmmintrin.h>
#include <immintrin.h>

pthread_barrier_t barr_worker;
pthread_barrier_t barr_continue;

int done = 0;

double * g_rankScoresPrev;
double * g_rankScoresCur;
double * g_rankScoresOutl;
double * g_rankPrevExp;
double * g_inPageOutL;

int * g_inPageIndex;
int * g_linksList;
int * g_index;
int * g_indexPagesLinks;

int g_ncores;
int g_npages;
int g_nedges;
int g_pad = 0;
double g_dampener;
double g_constant;

int g_tcount = 0;
pthread_spinlock_t g_lock;

typedef struct {
    double locVal;
    int start, end, index;
    char padding[40];
} wargs;

void * pr_worker(void * arg) {

    wargs * pr = (wargs *)arg;
    double dampener = g_dampener;
    double constant = g_constant;
    int start = pr->start, end = pr->end;
    int end2 = (end-start)*0.25;
    int * inPageIndex = g_indexPagesLinks + g_npages;
    int * linksList = g_indexPagesLinks + g_npages + g_nedges;
    
    /*Loop infinitely. This will stop running automatically at convergence*/
    for(;;) {
        if (done == 1)
            return NULL;
        int index = pr->index;
        for (int i = start; i < end; i+=12) {

            double curRank = 0;
            for (int j = 0; j < linksList[i]; j++)
            {
                curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                index++;
            }
            curRank = g_rankScoresCur[i] = constant + dampener * curRank;

            curRank = 0;
            for (int j = 0; j < linksList[i+1]; j++)
            {
                curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                index++;
            }
            curRank = g_rankScoresCur[i+1] = constant + dampener * curRank;

            curRank = 0;
            for (int j = 0; j < linksList[i+2]; j++)
            {
                curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                index++;
            }
            curRank = g_rankScoresCur[i+2] = constant + dampener * curRank;

            curRank = 0;
            for (int j = 0; j < linksList[i+3]; j++)
            {
                curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                index++;
            }
            curRank = g_rankScoresCur[i+3] = constant + dampener * curRank;

            curRank = 0;
            for (int j = 0; j < linksList[i+4]; j++)
            {
                curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                index++;
            }
            curRank = g_rankScoresCur[i+4] = constant + dampener * curRank;

            curRank = 0;
            for (int j = 0; j < linksList[i+5]; j++)
            {
                curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                index++;
            }
            curRank = g_rankScoresCur[i+5] = constant + dampener * curRank;

            curRank = 0;
            for (int j = 0; j < linksList[i+6]; j++)
            {
                curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                index++;
            }
            curRank = g_rankScoresCur[i+6] = constant + dampener * curRank;

            curRank = 0;
            for (int j = 0; j < linksList[i+7]; j++)
            {
                curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                index++;
            }
            curRank = g_rankScoresCur[i+7] = constant + dampener * curRank;

            curRank = 0;
            for (int j = 0; j < linksList[i+8]; j++)
            {
                curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                index++;
            }
            curRank = g_rankScoresCur[i+8] = constant + dampener * curRank;

            curRank = 0;
            for (int j = 0; j < linksList[i+9]; j++)
            {
                curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                index++;
            }
            curRank = g_rankScoresCur[i+9] = constant + dampener * curRank;

            curRank = 0;
            for (int j = 0; j < linksList[i+10]; j++)
            {
                curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                index++;
            }
            curRank = g_rankScoresCur[i+10] = constant + dampener * curRank;

            curRank = 0;
            for (int j = 0; j < linksList[i+11]; j++)
            {
                curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                index++;
            }
            curRank = g_rankScoresCur[i+11] = constant + dampener * curRank;
        }

        double eucValue = 0;
        __m256d * a = (__m256d*)(g_rankScoresPrev+start);
        __m256d * b = (__m256d*)(g_rankScoresCur+start);
        for (int i = 0; i < end2; ++i)
        {
            __m256d c = _mm256_sub_pd(a[i], b[i]);
            c = _mm256_mul_pd(c, c);
            eucValue += c[0] + c[1] + c[2] + c[3];
        }
        
        pr->locVal = eucValue;

        while(pthread_spin_lock(&g_lock) != 0);
        g_tcount++;
        pthread_spin_unlock(&g_lock);

        while (g_tcount != 0);
    }
    return NULL;
}

static void pagerank(list* plist, int ncores, int npages, int nedges, double dampener) {
    
    while ((npages + g_pad) % (ncores * 12) != 0)
        g_pad++;
    
    g_dampener = dampener;
    g_constant = (1 - dampener)/npages;

    if (npages < 500)
        ncores = 1;
    g_ncores = ncores;

    g_npages = npages;
    g_nedges = nedges;

    /*Create array of page structs, as well as
    2D array of inlink page structs for fast access*/
    char namesList[npages][21];

    posix_memalign((void*)&g_indexPagesLinks, 32, (npages + nedges + npages + g_pad) * sizeof(int));
    posix_memalign((void*)&g_rankScoresOutl, 32, (npages + g_pad + npages + g_pad + nedges) * sizeof(double));

    g_rankScoresPrev = g_rankScoresOutl;    
    g_rankScoresCur = g_rankScoresOutl + npages + g_pad;
    g_inPageOutL = g_rankScoresOutl + npages + g_pad + npages + g_pad;
    g_index = g_indexPagesLinks + npages + nedges;
    g_linksList = g_indexPagesLinks + npages;

    double initialScore = 1.0/npages;
    
    int ranges[ncores];
    ranges[0] = 0;
    ranges[ncores-1] = npages;
    int workload = (nedges)/ncores;
    int curWork = 0;
    int curIndex = 1;

    /*Store all necessary information in global, "combined" arrays for fast access*/
    int index = 0;
    node * curNode = plist->head;
    for (int i = 0; i != npages; ++i) {
        strcpy(namesList[i], curNode->page->name);

        /*Fill initial array with default pagerank scores*/
        g_rankScoresPrev[i] = initialScore;
        
        if(curNode->page->inlinks != NULL)
            g_index[i] = curNode->page->inlinks->length;
        else
            g_index[i] = 0;

        g_indexPagesLinks[i] = index;
        if (g_index[i] > 0) {

            /*Store page indexes and noutlinks of each page's inlinks*/
            int links = curNode->page->inlinks->length;
            
            node *inlistNode = curNode->page->inlinks->head;
            for (int j = 0; j < links; j++) {
                g_linksList[index] = inlistNode->page->index;

                /*Store reciprocals of noutlinks (constant) - no division in main function*/
                g_inPageOutL[index++] = 1/(double)inlistNode->page->noutlinks;
                inlistNode = inlistNode->next;
            }
            
            curWork += links;
            if (curWork >= workload && (i+1)%12 == 0 && curIndex != ncores-1) {
                curWork = 0;
            }

        }
        curNode = curNode->next;
    }

    /*for (int i = 0; i < ncores; ++i)
        printf("% d", ranges[i]);
    puts("");
    exit(0);*/

    /*Wipe data in padded areas of arrays*/
    for (int i = 0; i < g_pad; ++i)
    {
        g_index[npages+i] = 0;
        g_rankScoresPrev[npages+i] = 0;
        g_rankScoresCur[npages+i] = 0;
    }

    pthread_spin_init(&g_lock, 0);

    pthread_t threads[ncores-1];
    wargs pr[ncores-1];
    int splitter = (npages+g_pad)/ncores;
    for (int i = 0; i < ncores-1; ++i) {
        pr[i].start = i * splitter;
        pr[i].end = (i+1) * splitter;
        // printf("%d ", pr[i].start);
        // pr[i].start = ranges[i];
        // pr[i].end = ranges[i+1];
        pr[i].index = g_indexPagesLinks[pr[i].start];
    }

    /*Range setting for main thread worker*///Section confirmed as correct!
    int start = (ncores-1) * splitter;
    int end = npages+g_pad;
    
    for (int i = 0; i < ncores-1; ++i)
        pthread_create(&threads[i], NULL, pr_worker, &pr[i]);

    /*Check for e^2 rather than e to remove "sqrt" operation*/
    double modifiedEps = EPSILON * EPSILON;
    int * inPageIndex = g_indexPagesLinks + g_npages;
    int * linksList = g_indexPagesLinks + g_npages + g_nedges;
    
    double constant = g_constant;
    int end2 = (end-start)*0.25;
    int indexStart = g_indexPagesLinks[start];

    if (ncores > 1)
    {
        for(;;) {
            /*Use main thread as the worker thread for the "final" block*/
            int index = indexStart;
            
            for (int i = start; i < end; i+=12) {

                double curRank = 0;
                for (int j = 0; j < linksList[i]; j++)
                {
                    curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                    index++;
                }
                curRank = g_rankScoresCur[i] = constant + dampener * curRank;

                curRank = 0;
                for (int j = 0; j < linksList[i+1]; j++)
                {
                    curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                    index++;
                }
                curRank = g_rankScoresCur[i+1] = constant + dampener * curRank;

                curRank = 0;
                for (int j = 0; j < linksList[i+2]; j++)
                {
                    curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                    index++;
                }
                curRank = g_rankScoresCur[i+2] = constant + dampener * curRank;

                curRank = 0;
                for (int j = 0; j < linksList[i+3]; j++)
                {
                    curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                    index++;
                }
                curRank = g_rankScoresCur[i+3] = constant + dampener * curRank;

                curRank = 0;
                for (int j = 0; j < linksList[i+4]; j++)
                {
                    curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                    index++;
                }
                curRank = g_rankScoresCur[i+4] = constant + dampener * curRank;

                curRank = 0;
                for (int j = 0; j < linksList[i+5]; j++)
                {
                    curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                    index++;
                }
                curRank = g_rankScoresCur[i+5] = constant + dampener * curRank;

                curRank = 0;
                for (int j = 0; j < linksList[i+6]; j++)
                {
                    curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                    index++;
                }
                curRank = g_rankScoresCur[i+6] = constant + dampener * curRank;

                curRank = 0;
                for (int j = 0; j < linksList[i+7]; j++)
                {
                    curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                    index++;
                }
                curRank = g_rankScoresCur[i+7] = constant + dampener * curRank;

                curRank = 0;
                for (int j = 0; j < linksList[i+8]; j++)
                {
                    curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                    index++;
                }
                curRank = g_rankScoresCur[i+8] = constant + dampener * curRank;

                curRank = 0;
                for (int j = 0; j < linksList[i+9]; j++)
                {
                    curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                    index++;
                }
                curRank = g_rankScoresCur[i+9] = constant + dampener * curRank;

                curRank = 0;
                for (int j = 0; j < linksList[i+10]; j++)
                {
                    curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                    index++;
                }
                curRank = g_rankScoresCur[i+10] = constant + dampener * curRank;

                curRank = 0;
                for (int j = 0; j < linksList[i+11]; j++)
                {
                    curRank += g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                    index++;
                }
                curRank = g_rankScoresCur[i+11] = constant + dampener * curRank;
            }
            
            /*Calculate local convergence using AVX 256 bit registers*/
            double eucValue = 0;
            
            __m256d * a = (__m256d*)(g_rankScoresPrev+start);
            __m256d * b = (__m256d*)(g_rankScoresCur+start);
            for (int i = 0; i < end2; ++i)
            {
                __m256d c = _mm256_sub_pd(a[i], b[i]);
                c = _mm256_mul_pd(c, c);
                eucValue += c[0] + c[1] + c[2] + c[3];
            }

            while (g_tcount != ncores-1);

            /*Swap current and previous pagerank vectors*/
            double * tmp = g_rankScoresPrev;
            g_rankScoresPrev = g_rankScoresCur;
            g_rankScoresCur = tmp;

            /*Find sum to check for convergence*/
            for (int i = 0; i < ncores-1; ++i)
                eucValue += pr[i].locVal;

            g_tcount = 0;
            if (eucValue <= modifiedEps)
            {
                done = 1;
                g_tcount = 0;
                break;
            }
        }
    }
    
    /*For smaller cases where code for larger parallelism cases messes up...
    a rather hacky solution @___@*/
    else
    {
        for(;;) {
            /*Use main thread as the worker thread for the "final" block*/
            int index = g_indexPagesLinks[start];
            double eucValue = 0;
            for (int i = start; i < end; ++i)
            {
                if (linksList[i] > 0) {
                    double curRank = 0;
                    for(int j = 0; j < linksList[i]; j++) {

                        curRank+= g_rankScoresPrev[inPageIndex[index]] * g_inPageOutL[index];
                        index++;
                    }
                    curRank = g_rankScoresCur[i] = constant + dampener * curRank;
                    eucValue += (curRank - g_rankScoresPrev[i]) * (curRank - g_rankScoresPrev[i]);
                }
                else {
                    g_rankScoresCur[i] = constant;
                    eucValue += (constant - g_rankScoresPrev[i]) * (constant - g_rankScoresPrev[i]);
                } 
            }
            /*Swap current and previous pagerank vectors*/
            double * tmp = g_rankScoresPrev;
            g_rankScoresPrev = g_rankScoresCur;
            g_rankScoresCur = tmp; 

            if (eucValue <= modifiedEps)
                break;
        }
    }

    char buf[300000];
    setbuf(stdout, buf);
    /*Print final PR values at convergence*/
    for (int i = 0; i != npages; ++i)
        printf("%s %.4lf\n", namesList[i], g_rankScoresPrev[i]);

    /*Free allocated memory*/
    free(g_rankScoresOutl);
    free(g_indexPagesLinks);    
    pthread_barrier_destroy(&barr_continue);
    pthread_spin_destroy(&g_lock);

    for (int i = 0; i < ncores-1; ++i)
        pthread_join(threads[i], NULL);
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
