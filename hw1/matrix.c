#ifndef _REENTRANT 
#define _REENTRANT 
#endif 
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>

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
int partial_sums[MAXWORKERS];
int matrix[MAXSIZE][MAXSIZE];

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
    return end.tv_usec - start.tv_sec;
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
    start_time = read_timer();
    for(int w = 0; w < num_workers; w++){
        pthread_create(&worker_id[w],&attr, Worker, (void *) w);
    }

    pthread_exit(NULL); // should return 0 if success
}

// routine for threads
/*  Each worker performs work on one strip of matrix
    After reachiing barrier, worker(0) computes and prints the total */
void *Worker(void *arg){
    // TODO finish
}