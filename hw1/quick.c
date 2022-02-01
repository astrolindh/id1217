
/*
Enkel implementation av quicksort, sekventiell
*/

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

// timer
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

int main(int argc, char *argv[]){
    // read input arguments from command line, or set to standard values
    size = (argc > 1)? atoi(argv[1]) : STANDARDSIZE;
    if(size > MAXSIZE){size = STANDARDSIZE;}

    // create the array to be sorted, populate with values of [0,98]
    for(int i = 0; i <= size; i++){ unsorted[i] = rand()%99; }
    #ifdef DEBUG
        printf("{");
        for(int i = 0; i <= size; i++){
            printf("%d, ", unsorted[i]);
        }
        printf("}\n");
    #endif

    start_time = read_timer();
    quick(unsorted, 0, size -1);
    end_time = read_timer();
    #ifdef DEBUG
        printf("{");
        for(int i = 0; i < size; i++){
            printf("%d, ", unsorted[i]);
        }
        printf("}\n");
    #endif
    printf("The execution time is %g seconds\n", end_time - start_time);
}