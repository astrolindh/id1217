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

#define MAXSIZE 100000       // max size of input array to be sorted
#define STANDARDSIZE 100   // if no command line argument to specify input size, use this value
#define MAXWORKERS 10           // change to 15 for first four levels being new threads only
#define STANDARDWORKERS 10   // if no number of workers specified at command line, use this value

// shared and global variables
pthread_mutex_t lock;       // synchronises incrementation of created_workers
double start_time, end_time;
long size;                      // n values to be sorted
int num_workers;            // maximum allowed number of workers
int created_workers;        // current number of workers
pthread_t workers[MAXWORKERS + 1];
int values[MAXSIZE];
struct limits {
    long first, last;       // first and last positions for a quicksort partition
    int my_id;              // levels of partioning, to limit thread spawning
};

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

// non paralellized quicksort - utilized by threads that cannot create further threads
void quick(int values[], int first, int last){
    int i, j, pivot, temp;
    if(first<last){
        pivot = first;
        i = first;
        j = last;
        while(i < j){
            while(values[i] <= values[pivot] && i < last) { i++; }
            while(values[j] > values[pivot]) { j--; }
            if(i < j){
                temp = values[i];
                values[i] = values[j];
                values[j] = temp;
            }
        }
        temp = values[pivot];
        values[pivot] = values[j];
        values[j] = temp;
        quick(values, first, j-1);  // sort left subsection
        quick(values, j+1, last);   // sort right subsection
    }
}


void *Quick(void *l){
    // long myid = (long) arg;
    int i, j, pivot, temp, first, last, my_id;

    // todo, objektet som tas emot är en limits, ur denna hämtas first, last, my_id
    struct limits *lim = l;
    my_id = lim->my_id;
    first = lim->first;
    last = lim->last;

    // if my_id < num_workers
    printf("worker %d is starting\n", my_id);
    if(first < last){
        pivot = first;
        i = first;
        j = last;
        while(i < j){
            while(values[i] <= values[pivot] && i < last) { i++; }
            while(values[j] > values[pivot]){ j--; }
            if(i < j){
                temp = values[i];
                values[i] = values[j];
                values[j] = temp;
            }
        }
        temp = values[pivot];
        values[i] = values[j];
        values[j] = temp;

        // left partition
        // CS START

        // attempt spinning new thread for left partition
        pthread_mutex_lock(&lock);
        if(created_workers < num_workers){

            // set up for new thread
            created_workers++;
            int new_id = created_workers;
            pthread_mutex_unlock(&lock);
            // critical section end
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            struct limits point = {0, j-1, new_id};     // left partition: between first and j-1
            struct limits *partition_limits;
            partition_limits = &point;
            printf("Thread %d, starts partition at %ld, %ld\n", 
                partition_limits->my_id, 
                partition_limits->first, 
                partition_limits->last);
            pthread_create(&workers[new_id], &attr, Quick, (void *) partition_limits);
        }
        else{
            pthread_mutex_unlock(&lock);
            // go to standard quicksort for left partition
            quick(values, first, j-1);
        }
        pthread_mutex_lock(&lock);
        if(created_workers < num_workers){

            // set up for new thread
            created_workers++;
            int new_id = created_workers;
            pthread_mutex_unlock(&lock);
            // critical section end
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            struct limits point = {j+1, last, new_id};     // left partition: between first and j-1
            struct limits *partition_limits;
            partition_limits = &point;
            printf("Thread %d, starts partition at %ld, %ld\n", 
                partition_limits->my_id, 
                partition_limits->first, 
                partition_limits->last);
            pthread_create(&workers[new_id], &attr, Quick, (void *) partition_limits);
        }
        else{
            pthread_mutex_unlock(&lock);
            // go to standard quicksort for left partition
            quick(values, j+1, last);
        }

        // let new thread sort right subsection
        // let old thread sort left subsection, and wait for new to join
        // pthread_exit, in order to let parent thread accept this child?
    }
    // this should only be reached when all else is resolved
    // pthread_exit returning only null (since array is sorted, no return values needed?)
    pthread_exit(NULL);
}

int main(int argc, char *argv[]){
    // read input arguments from command line, or set to standard values
    size = (argc > 1)? atol(argv[1]) : STANDARDSIZE;
    if(size > MAXSIZE){size = STANDARDSIZE;}
    num_workers = (argc > 2)? atoi(argv[2]) : STANDARDWORKERS;
    if(num_workers > MAXWORKERS){num_workers = STANDARDWORKERS;}

    pthread_attr_t attr;
    // pthread_t workers[num_workers];
    pthread_t *restrict root_worker;
    // pthread_t *rw;
    // w = &root_worker;
    long w = 0;
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, PTHREAD_SCOPE_SYSTEM);
    pthread_mutex_init(&lock, NULL);

    // create the array to be sorted, populate with values of [0,98]
    for(long i = 0; i <= size; i++){ values[i] = rand()%99; }
    // for(long j = 0; j <= num_workers; j++){ workers[j]; }
    #ifdef DEBUG
        printf("{");
        for(int i = 0; i < size; i++){
            printf("%d, ", values[i]);
        }
        printf("}\n");
    #endif
    start_time = read_timer();

    struct limits point = {0, size - 1, 0};     // first and last indeces of input array, first thread id
    struct limits *starting_point;
    starting_point = &point;
    printf("given starting point: %ld, %ld,  at my_id %d\n", starting_point->first, starting_point->last, starting_point->my_id);
    created_workers = 1;
        
    start_time = read_timer();
    pthread_create(&workers[created_workers], &attr, Quick, (void *) starting_point);
    

    end_time = read_timer();
    #ifdef DEBUG
        printf("{");
        for(int i = 0; i < size; i++){
            printf("%d, ", values[i]);
        }
        printf("}\n");
    #endif
    printf("Worker %ld finished\n", w);
    printf("The execution time is %g seconds\n", end_time - start_time);
}
