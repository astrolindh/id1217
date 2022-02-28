/*
Homework assignement 1, ID1217 VT22
Astrid Lindh

    Task 1B Matrix elements.
    Compute total sum of all elements. Find the minimum and maximum value elements, 
    print out the total sum as well as min, max with indeces.
    Solve without using a barrier (pthread_exit and pthread_join)

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
#include <string.h>

#define MAXSIZE 10000       // maximum matrix size
#define STANDARDSIZE 100    // matrix size, if not specified
#define MAXWORKERS 10       // maximum number of workers
#define STANDARDWORKERS 10  // number of workers, if not specified

// shared variables
pthread_mutex_t lock;       // mutex lock to read/write shared variables
int num_workers;           /* number of workers */ 
double start_time, end_time;
int size, strip_size;
int matrix[MAXSIZE][MAXSIZE];
int gmin = 100;
int gmax = -1;
int gmin_i, gmin_j, gmax_i, gmax_j; // global minimum and maximum, and their corresponding indeces

void *Worker(void *);       // proper definition after main

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

int main(int argc, char *argv[]){
    // attempt to read command line arguments, check that arguments are within bounds
    size = (argc > 1)? atoi(argv[1]) : STANDARDSIZE;
    if(size > MAXSIZE){size = STANDARDSIZE;}
    num_workers = (argc > 2)? atoi(argv[2]) : STANDARDWORKERS;
    if(num_workers > MAXWORKERS){num_workers = STANDARDWORKERS;}

    // set strip size
    strip_size = size/num_workers;

    // initialize local variables
    int m, n;   
    pthread_attr_t attr;                // NULL?
    pthread_t worker_id[num_workers];   // array of all workers

    // global thread attributes (necessary?)
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, PTHREAD_SCOPE_SYSTEM);

    pthread_mutex_init(&lock, NULL);

    // initialize matrix
    for(m = 0; m < size; m++){
        for(n = 0; n < size; n++){
            matrix[m][n] = rand()%99;
        }
    }

    // print the matrix
    #ifdef DEBUG
    for (int i = 0; i < size; i++) {
	    printf("[ ");
	    for (int j = 0; j < size; j++) {
	        printf(" %d", matrix[i][j]);
	    }
	    printf(" ]\n");
    }
    #endif

    // PARALELL START, create workers
    int *results;
    results = (int*)calloc(1, sizeof(int));
    int matrix_total = 0;

    start_time = read_timer();
    for(int w = 0; w < num_workers; w++){
        pthread_create(&worker_id[w],&attr, Worker, (void *) w);
    }
    for(int w = 0; w < num_workers; w++){
        pthread_join(worker_id[w], (void*) results);
        matrix_total += *results;
    }
    end_time = read_timer();
    printf("final total === %d\n", matrix_total);
    printf("one global max is: %d  found at (%d,%d)\n", gmax, gmax_i, gmax_j);
    printf("one global min is: %d  found at (%d,%d)\n", gmin, gmin_i, gmin_j);
    printf("The execution time is %g seconds\n", end_time - start_time);
    pthread_exit(NULL);
}

//  A. Worker should find maximum and minimum values, with corresponding indeces.
//      Worker 0 should select the global max and min and print out their indeces
void *Worker(void *arg){
    long myid = (long) arg;
    int total, i, j, first, last;
    int min, max, min_i, min_j, max_i, max_j;

    #ifdef DEBUG
        printf("worker %ld is starting\n", myid);
    #endif
    // determine first and last rows in my strip
    first = myid*strip_size;
    last = (myid == num_workers -1)? (size - 1) : (first + strip_size -1);

    // sum all values in my strip
    total = 0;
    for(i = first; i<=last; i++){
        for(j = 0; j < size; j++){
            total += matrix[i][j];
        }
    }
    
    // find minimum value and index in my strip. If several min values, selects the latest occurence of min or mac in strip
    // valid values in matrix is [0,98]
    min = 100;
    max = -1;
    for(i = first; i<=last; i++){
        for(j = 0; j < size; j++){
            if(matrix[i][j] < min){
                min = matrix[i][j];
                min_i = i;
                min_j = j;
            }
            if(matrix[i][j] > max){
                max = matrix[i][j];
                max_i = i;
                max_j = j;
            }
        }
    }
    #ifdef DEBUG
        printf("worker %ld found this local max: %d at index (%d, %d)\n", myid, max, max_i, max_j);        
        printf("worker %ld found this local min: %d at index (%d, %d)\n", myid, min, min_i, min_j);
        // printf("worker %ld reached barrier\n", myid);
    #endif

    // compare local thread min and max to global matrix min, max.
    // save to thread-external data structure if lesser than current global min, larger than current global max
    pthread_mutex_lock(&lock); 
    if(min < gmin){
        gmin = min;
        gmin_i = min_i;
        gmin_j = min_j;
    }
    if(max > gmax){
        gmax = max;
        gmax_i = max_i;
        gmax_j = max_j;
    }
    pthread_mutex_unlock(&lock);

    #ifdef DEBUG
        printf("attempting exit of worker %ld\n", myid);
    #endif

    pthread_exit((void*) total);
}