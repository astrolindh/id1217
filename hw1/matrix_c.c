#ifndef _REENTRANT 
#define _REENTRANT 
#endif 
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>

#define MAXSIZE 10000       // maximum matrix size
#define STANDARDSIZE 100    // matrix size, if not specified
#define MAXWORKERS 10       // maximum number of workers
#define STANDARDWORKERS 10  // number of workers, if not specified

// shared variables
pthread_mutex_t lock;       // mutex lock, to read/write shared variables
pthread_mutex_t bag;        // mutex lock, to synchronize bag of tasks task assignment
int num_workers;            // number of workers
double start_time, end_time;
int size;                   // matrix is of size*size
int matrix[MAXSIZE][MAXSIZE];
int gmin = 100;
int gmax = -1;
int gmin_i, gmin_j, gmax_i, gmax_j; // global minimum and maximum, and their corresponding indeces

int next_row = 0;           // bag of tasks
int gtotal = 0;          // sum of all matrix elements, global total

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

    // initialize local variables
    int m, n;   
    pthread_attr_t attr;                // NULL?
    pthread_t worker_id[num_workers];   // array of all workers

    // global thread attributes (necessary?)
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, PTHREAD_SCOPE_SYSTEM);

    pthread_mutex_init(&lock, NULL);
    pthread_mutex_init(&bag, NULL);

    // initialize matrix
    for(m = 0; m < size; m++){
        for(n = 0; n < size; n++){
            matrix[m][n] = rand()%99;
        }
    }

    // PARALELL START, create workers
    start_time = read_timer();
    for(int w = 0; w < num_workers; w++){
        pthread_create(&worker_id[w],&attr, Worker, (void *) w);
    }
    for(int w = 0; w < num_workers; w++){
        pthread_join(worker_id[w], (void*) NULL);
    }
    end_time = read_timer();
    printf("final total === %d\n", gtotal);
    printf("one global max is: %d  found at (%d,%d)\n", gmax, gmax_i, gmax_j);
    printf("one global min is: %d  found at (%d,%d)\n", gmin, gmin_i, gmin_j);
    printf("The execution time is %g seconds\n", end_time - start_time);
    pthread_exit(NULL);
}

void *Worker(void *arg){
    long myid = (long) arg;
    int total, i, j;
    int min, max, min_i, min_j, max_i, max_j;
    int row;    // the current task
    #ifdef DEBUG
        printf("worker %ld is starting\n", myid);
    #endif
    

    while(true){
        // CS grab task
        pthread_mutex_lock(&bag);
        row = next_row;
        next_row++;
        #ifdef DEBUG
            printf("worker %ld grabbed row %d\n", myid, row);
        #endif
        pthread_mutex_unlock(&bag);
        
        if(row >= size){ 
            #ifdef DEBUG
                printf("worker %ld breaks at row %d\n", myid, row);
            #endif
            break; 
        }  

        min = 100;
        max = -1;
        total = 0;
        for(j = 0; j < size; j++){
            total += matrix[row][j];
            if(matrix[row][j] < min){
                min = matrix[row][j];
                min_i = row;
                min_j = j;
            }
            if(matrix[row][j] > max){
                max = matrix[row][j];
                max_i = row;
                max_j = j;
            }
        }
        // critical section: store total, conditional store on min, max
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
        gtotal += total;
        pthread_mutex_unlock(&lock);
    }
    #ifdef DEBUG
        printf("attempting exit of worker %ld\n", myid);
    #endif

    pthread_exit((void*) NULL);
}