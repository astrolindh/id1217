/*
    Attempt at paralellizing quicksort, using recursive paralellism.
*/

#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>

#define MAXSIZE 1000       // max size of input array to be sorted
#define STANDARDSIZE 100   // if no command line argument to specify input size, use this value
#define MAXWORKERS 10
#define STANDARDWORKERS 10   // if no number of workers specified at command line, use this value

// shared and global variables
double start_time, end_time;
int size;                      // n values to be sorted
int num_workers;            // number of workers
int unsorted[MAXSIZE];


// timer for paralell code section
double read_timer(){
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if(!initialized){
        gettimeofday(&start, NULL);
        initialized = true;
    }
    gettimeofday(&end, NULL);
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}


void *Quick(void *arg, int numbers[], int first, int last){
    long myid = (long) arg;
    int i, j, pivot, temp;

    printf("worker %ld is starting\n", myid);
}

int main(int argc, char *argv[]){
    // read input arguments from command line, or set to standard values
    size = (argc > 1)? atoi(argv[1]) : STANDARDSIZE;
    if(size > MAXSIZE){size = STANDARDSIZE;}
    num_workers = (argc > 2)? atoi(argv[2]) : STANDARDWORKERS;
    if(num_workers > MAXWORKERS){num_workers = STANDARDWORKERS;}

    pthread_attr_t attr;
    pthread_t workers[num_workers];
    long w = 0;
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, PTHREAD_SCOPE_SYSTEM);

    // create the array to be sorted, populate with values of [0,98]
    for(int i = 0; i <= size; i++){ unsorted[i] = rand()%99; }
    for(long j = 0; j <= num_workers; j++){ workers[j]; }
    #ifdef DEBUG
        printf("{");
        for(int i = 0; i < size; i++){
            printf("%d, ", unsorted[i]);
        }
        printf("}\n");
    #endif
    start_time = read_timer();

    long w2 = 1;
    pthread_create(&workers[w], &attr, Quick, (void *) w);
    pthread_create(&workers[w2], &attr, Quick, (void *) w2);


    end_time = read_timer();
    #ifdef DEBUG
        printf("{");
        for(int i = 0; i < size; i++){
            printf("%d, ", unsorted[i]);
        }
        printf("}\n");
    #endif
    printf("Worker %ld finished\n", w);
    printf("The execution time is %g seconds\n", end_time - start_time);
}
