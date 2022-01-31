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

#define MAXSIZE 100       // maximum matrix size
#define STANDARDSIZE 100    // matrix size, if not specified
#define MAXWORKERS 10       // maximum number of workers
#define STANDARDWORKERS 10  // number of workers, if not specified

pthread_mutex_t barrier;  /* mutex lock for the barrier */
pthread_cond_t go;        /* condition variable for leaving */
int num_workers;           /* number of workers */ 
int num_barrier = 0;       /* number who have arrived */

double start_time, end_time;
int size, strip_size;
// int partial_sums[MAXWORKERS];
int matrix[MAXSIZE][MAXSIZE];

// stores local max- and min values, with corresponding indeces
// int partial_max[MAXWORKERS];
// char partial_max_ind[MAXWORKERS][8*2 + 5];
// int partial_min[MAXWORKERS];
// char partial_min_ind[MAXWORKERS][8*2 + 5];

void *Worker(void *);       // proper definition after main

// reusable counter barrier
void Barrier(){
    pthread_mutex_lock(&barrier);   // CS entry
    num_barrier++;
    if(num_barrier == num_workers){
        num_barrier = 0;           // reset counter
        pthread_cond_broadcast(&go);
        pthread_mutex_unlock(&barrier);     // CS exit (unnecessary?)
    }
    else{
        pthread_cond_wait(&go, &barrier);
        pthread_mutex_unlock(&barrier);     // CS exit
    }
}

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
    // return end.tv_usec - start.tv_sec;
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

    // global thread ttributes (necessary?)
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, PTHREAD_SCOPE_SYSTEM);

    // initialize mutex, condition variable
    pthread_mutex_init(&barrier, NULL);
    pthread_cond_init(&go, NULL);

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
        printf("joining %d to main thread\n", w);
        pthread_join(worker_id[w], (void*) results);
        matrix_total += *results;
        printf ("Completed join with thread %d. Result =%d\n", w, *results);
        // TODO iterera över alla joinade barntrådar, hämta in info från results
    }
    end_time = read_timer();
    printf("final total === %d\n", matrix_total);
    printf("The execution time is %g seconds\n", end_time - start_time);
    pthread_exit(NULL);
}

// routine for threads
/*  Each worker performs work on one strip of matrix
    After reachiing barrier, worker(0) computes and prints the total */

//  A. Worker should find maximum and minimum values, with corresponding indeces.
//      Worker 0 should select the global max and min and print out their indeces
void *Worker(void *arg){
    long myid = (long) arg;
    int total, i, j, first, last;
    int min, max, min_i, min_j, max_i, max_j;
    int partial_result[7];

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
    
    // find minimum value and index in my strip
    // valid values in matrix is [1,99]
    // min = __INT_MAX__;
    min = 100;
    max = -1;
    for(i = first; i<=last; i++){
        for(j = 0; j < size; j++){
            if(matrix[i][j] < min){
                min = matrix[i][j];
                min_i = i;
                min_j = j;
            }
            else if(matrix[i][j] > max){
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

    partial_result[0] = total;
    partial_result[1] = max;
    partial_result[2] = max_i;
    partial_result[3] = max_j;
    partial_result[4] = min;
    partial_result[5] = min_i;
    partial_result[6] = min_j;

    #ifdef DEBUG
        printf("attempting exit of worker %ld\n", myid);
    #endif
    int* exit = (int*) 1;
    pthread_exit((void*) total);
}