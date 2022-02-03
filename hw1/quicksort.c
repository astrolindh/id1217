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
#define TRUE 1
#define FALSE 0

// shared and global variables
pthread_mutex_t lock;       // synchronises incrementation of created_workers
double start_time, end_time;
long size;                      // n values to be sorted
int num_workers;            // maximum allowed number of workers
int created_workers;        // current number of workers
pthread_t workers[MAXWORKERS + 1];      // because indexed at 1, that's why
int values[MAXSIZE];
struct limits {
    long first, last;       // first and last positions for a quicksort partition
    int my_id;              // this thread
    bool left_exists, right_exists;   // presence of a child thread dealing with left or right side partition
    pthread_t left, right;       // id of left, right child.
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
    printf("ENTERED QUICK\n");
    // long myid = (long) arg;
    int i, j, pivot, temp, my_id;
    long first, last;
    bool left_exists, right_exists;
    pthread_t left_id, right_id;
    // todo, objektet som tas emot är en limits, ur denna hämtas first, last, my_id
    struct limits *lim = l;
    my_id = lim->my_id;
    first = lim->first;
    last = lim->last;
    left_exists = lim->left_exists;
    left_id = lim->left;
    right_exists = lim->right_exists;
    right_id = lim->right;
    

    printf("reading from struct     first: %ld, last: %ld.  my_id: %d\n", first, last, my_id);
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

        // attempt to spin off thread to deal with the left side partition
        // can I spin off a thread to do the left partition?
        // if not, attempt to spin a new thread and add it as the left child thread
        if(!left_exists){
            pthread_mutex_lock(&lock);
            if(created_workers < num_workers){

                // set up for new thread
                created_workers++;
                int new_id = created_workers;
                pthread_mutex_unlock(&lock);
                // critical section end

                pthread_attr_t attr;
                pthread_attr_init(&attr);
                struct limits point = {0, j-1, new_id, FALSE, FALSE, NULL, NULL};     // left partition: between first and j-1
                struct limits *partition_limits;
                partition_limits = &point;
                printf("Thread %d, starts partition at %ld, %ld\n", 
                    partition_limits->my_id, 
                    partition_limits->first, 
                    partition_limits->last);
                pthread_create(&workers[new_id], &attr, Quick, (void *) partition_limits);
                // update parent thread info on its left child
                lim->left_exists = TRUE;
                lim->left = &workers[new_id];
            }
            // no previous left child exists, but thread spawning is limited
            else{
                pthread_mutex_unlock(&lock);
                // go to standard quicksort for left partition
                printf("Thread %d can't spin new threads, standard quicksort\n", my_id);
                quick(values, first, j-1);
            }
            printf("no left exists\n");
        }
        // attempt to spin off a thread to deal with right side partition
        if(!right_exists){
            if(created_workers < num_workers){

                pthread_mutex_lock(&lock);
                // set up for new thread
                created_workers++;
                int new_id = created_workers;
                pthread_mutex_unlock(&lock);
                // critical section end
                pthread_attr_t attr;
                pthread_attr_init(&attr);
                struct limits point = {j+1, last, new_id, FALSE, FALSE, NULL, NULL};     // right partition: between j+1 and last
                struct limits *partition_limits;
                partition_limits = &point;
                printf("Thread %d, starts partition at %ld, %ld\n", 
                    partition_limits->my_id, 
                    partition_limits->first, 
                    partition_limits->last);
                pthread_create(&workers[new_id], &attr, Quick, (void *) partition_limits);
                // update parent thread with info on its right child
                lim->right_exists = TRUE;
                lim->right = &workers[new_id];
           }
           else{
                pthread_mutex_unlock(&lock);
               // go to standard quicksort for right partition
                printf("Thread %d can't spin new threads, standard quicksort\n", my_id);
                quick(values, j+1, last);
           }
        }        

        // IF NO THREAD CHILDREN COULD BE SPAWNED; GO TO STANDARD QUICKSORT
        if(left_exists){ quick(values, first, j-1); }    
        if(right_exists){ quick(values, j+1, last); }
    }
    // TODO: MUST join in its children (if any)
    // this should only be reached when all else is resolved
    // pthread_exit returning only null (since array is sorted, no return values needed?)
    // pthread_join();
    // how to find a thread's children? should there be something keeping track of left and right child
    if(lim->left_exists){ pthread_join(&lim->left, NULL); }
    if(lim->right_exists){ pthread_join(&lim->right, NULL); }
    
    printf("Thread %d exiting\n", lim->my_id);
    pthread_exit(NULL);
}


void sanity(){
    bool ordered = true;
    for(int i = 1; i <= size; i++){
        if(values[i-1] > values[i]){ ordered = false;}
    }
    if(ordered){ printf("\noutput is ordered\n\n"); }
    else { printf("\nNOT IN ORDER!\n\n"); }
}
int main(int argc, char *argv[]){
    // read input arguments from command line
    // if no arguments, set to standard values
    size = (argc > 1)? atol(argv[1]) : STANDARDSIZE;
    if(size > MAXSIZE){size = STANDARDSIZE;}
    num_workers = (argc > 2)? atoi(argv[2]) : STANDARDWORKERS;
    if(num_workers > MAXWORKERS){num_workers = STANDARDWORKERS;}
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, PTHREAD_SCOPE_SYSTEM);
    pthread_mutex_init(&lock, NULL);

    // create the array to be sorted, populate with values of [0,98]
    for(long i = 0; i <= size; i++){ values[i] = rand()%99; }
    #ifdef DEBUG
        /*printf("{");
        for(int i = 0; i < size; i++){
            printf("%d, ", values[i]);
        }
        printf("}\n");*/
    #endif
    created_workers = 0;
    // preparing for creation of first worker
    int first_worker = 1;
    created_workers++;
    struct limits point = {(long) 0, (long) size - 1, created_workers, FALSE, FALSE, NULL, NULL};     // first and last indeces of input array, first thread id
    struct limits *starting_point;
    starting_point = &point;
    printf("given starting point: %ld, %ld,  at my_id %d\n", starting_point->first, starting_point->last, starting_point->my_id);
    start_time = read_timer();
    // creating first worker
    pthread_create(&workers[first_worker], &attr, Quick, (void *) starting_point);
    
    
    printf("number of created_workers:   %d\n", created_workers);
    pthread_join(&workers[first_worker], NULL);

    end_time = read_timer();
    #ifdef DEBUG
        /*printf("{");
        for(int i = 0; i < size; i++){
            printf("%d, ", values[i]);
        }
        printf("}\n");*/
    #endif
    #ifdef DEBUG
        sanity();
    #endif
    printf("Main execution path finished\n");
    printf("The execution time is %g seconds\n", end_time - start_time);
}
