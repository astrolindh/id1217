/*
    Attempt at paralellizing quicksort, using recursive paralellism.
    ATTENTION! CODE DOES NOT WORK!
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

#define MAXSIZE 100000       //  max size of input array to be sorted
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
void quick(int values[], int first, int last, int thread){
    int i, j, pivot, temp;
    #ifdef DEBUG
    printf("quicksort by thread %d     first %d, last: %d\n", thread, first, last);
    #endif
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
        quick(values, first, j-1, thread);  // sort left subsection
        quick(values, j+1, last, thread);   // sort right subsection
    }
}

void *Quick(void *l){
    int i, j, pivot, temp, my_id;
    long first, last;
    bool left_exists, right_exists;
    pthread_t left_id, right_id;
    struct limits *lim = l;
    my_id = lim->my_id;
    first = lim->first;
    last = lim->last;
    left_exists = lim->left_exists;
    left_id = lim->left;
    right_exists = lim->right_exists;
    right_id = lim->right;
    printf("ENTERED QUICK thread %d.\n", my_id);


    // printf("thread %ld:     first %ld, last %ld\n", my_id, first, last);
    pivot = first;
    i = first;
    j = last;
    if(abs(last - first) < 10){
        printf("branch %d too small, exiting after standard quicksort\n", my_id);
        quick(values, first, last, my_id);
        pthread_exit(NULL);
    }
    else if(first < last){
        
        while(values[i] < values[j]){
            while(values[i] <= values[pivot] && i < last) { i++; }
            while(values[j] > values[pivot] && i < j){ j--; }
            if(i < j){
                temp = values[i];
                values[i] = values[j];
                values[j] = temp;
            }
        }
        printf("thread %ld:     i , j  %d = (%d)\n", my_id, i,values[i]);

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
                lim->left_exists = TRUE;
                lim->left = &workers[new_id];
                printf("new worker %d created, left child of %d\n", new_id, my_id);
                pthread_mutex_unlock(&lock);
                // critical section end

                pthread_attr_t attr;
                pthread_attr_init(&attr);
                struct limits point = {first, j-1, new_id, FALSE, FALSE, NULL, NULL};     // left partition: between first and j-1
                struct limits *partition_limits;
                partition_limits = &point;
                printf("sending off left child %d of parent %d\n", new_id, my_id);
                pthread_create(&workers[new_id], &attr, Quick, (void *) partition_limits);
                // update parent thread info on its left child
                
            }
            // no previous left child exists, but thread spawning is limited
            else{
                pthread_mutex_unlock(&lock);
                // go to standard quicksort for left partition
                printf("Thread %d can't spin new threads to the left, standard quicksort\n", my_id);
                quick(values, first, j, my_id);
            }
        }
        // attempt to spin off a thread to deal with right side partition
        if(!right_exists){
            if(created_workers < num_workers){

                pthread_mutex_lock(&lock);
                // set up for new thread
                created_workers++;
                int new_id = created_workers;
                lim->right_exists = TRUE;
                lim->right = &workers[new_id];
                printf("new worker %d created, right child of %d\n", new_id, my_id);
                pthread_mutex_unlock(&lock);
                // critical section end
                pthread_attr_t attr;
                pthread_attr_init(&attr);
                struct limits point = {j, last, new_id, FALSE, FALSE, NULL, NULL};     // right partition: between j+1 and last
                struct limits *partition_limits;
                partition_limits = &point;
                
                pthread_create(&workers[new_id], &attr, Quick, (void *) partition_limits);
                // update parent thread with info on its right child
                
           }
           else{
                pthread_mutex_unlock(&lock);
               // go to standard quicksort for right partition
                printf("Thread %d can't spin new threads to the right, standard quicksort\n", my_id);
                quick(values, j+1, last, my_id);
           }
        }        

        // IF NO THREAD CHILDREN COULD BE SPAWNED; GO TO STANDARD QUICKSORT
        if(left_exists){ quick(values, first, j-1, my_id); }    
        if(right_exists){ quick(values, j+1, last, my_id); }
    }
    
    printf("thread %ld waiting for joining children\n", my_id);

    if(lim->left_exists){ pthread_join(&lim->left, NULL); }
    if(lim->right_exists){ pthread_join(&lim->right, NULL); }
    #ifdef DEBUG
        printf("Thread %d ready to exit\n", lim->my_id);
    #endif
    pthread_exit(NULL);
}

void sanity(int values[]){
    int i = 0;
    int j = 1;
    bool ordered = true;
    for(int i = 1; i <size -1; i++){
        j++;
        if(values[i] > values[j]){ ordered = false;}
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
    for(long i = 0; i < size; i++){ values[i] = rand()%1000; }
    #ifdef DEBUG
        printf("{");
        for(int i = 0; i < size; i++){
            printf("%d, ", values[i]);
        }
        printf("}\n");
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
    printf("creating worker %d\n", first_worker);
    pthread_create(&workers[first_worker], &attr, Quick, (void *) starting_point);
    
    pthread_join(workers[first_worker], NULL);
    printf("joined\n");
    // quick(values, 0, size-1);
    end_time = read_timer();

    printf("number of created_workers:   %d\n", created_workers);

    #ifdef DEBUG
        /*printf("{");
        for(int i = 0; i < size; i++){
            printf("%d, ", values[i]);
        }
        printf("}\n");*/
    #endif
    #ifdef DEBUG
        sanity(values);
    #endif
    printf("Main execution path finished\n");
    printf("The execution time is %g seconds\n", end_time - start_time);
}
