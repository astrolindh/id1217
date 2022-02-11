/*

Homework assignement 2, ID1217 VT22
Astrid Lindh
    build and compile with
        gcc -o quick openquick.c -fopenmp


Code is inspired by but not copied from a source.
Code reference:
https://stackoverflow.com/questions/16007640/openmp-parallel-quicksort
answer by user dreamcrash


    The quicksort algorithm sorts the list of numbers by first dividing the list into two sublists,
 so that all the numbers if one sublist  are smaller than all the numbers in the other sublist.
  This is done by selecting one number (called a pivot) against which all other number are 
  compared: the numbers which are less than the pivot are placed in one sublist, the numbers 
  which more than the pivot are placed in another sublist. The pivot can be either placed in 
  one sublist or could be withheld and placed in its final position. 

  Run the program on different number of processors and report the speedup (sequential 
  execution time divided by parallel execution time) for different number of processors 
  (up to at least 4) and different workloads (at least 3 different lists of different sizes)
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <omp.h>

#define TASK_SIZE 10000
#define MAX_WORKERS 16
#define MAX_ELEMS 100000


int isSorted(int *arr, int size){
    for(int i = 1; i < size ; i++){
        if(arr[i-1] > arr[i]){ return 0;}
    }
    return 1;
}

int partition(int *arr, int pivot, int last){
    int lesser_than[last-pivot];
    int greater_than[last-pivot];
    int i, j;
    int key = arr[last];
    int ln = 0;
    int gn = 0;

    // find ints lesser than, greater than pivot
    for(i = pivot; i < last; i++){
        if(arr[i] < arr[last]){
            lesser_than[ln++] = arr[i];
        }
        else{
            greater_than[gn++] = arr[i];
        }
    }


    for(i = 0; i < ln; i++){
        arr[pivot + i] = lesser_than[i];
    }
    arr[pivot + ln] = key;
    for(j = 0; j < gn; j++){
        arr[pivot + ln + j + 1] = greater_than[j];
    }
    return pivot + ln;
}

void quicksort(int *arr, int pivot, int last){
    int div;
    if(pivot < last){
        div = partition(arr, pivot, last);
        #pragma omp task shared(arr) if(last - pivot > TASK_SIZE)
        quicksort(arr, pivot, div - 1);
        #pragma omp task shared(arr) if(last - pivot > TASK_SIZE)
        quicksort(arr, div + 1, last);
    }
}

int main(int argc, char *argv[]){
    int num_elems = (argv > 1) ? atoi(argv[1]) : 10;
    int num_workers = (argc > 2) ? atoi(argv[2]) : 8;
    // set max number of threads
    if(num_elems < 9 || num_elems > MAX_ELEMS) { num_elems = MAX_ELEMS; }
    if(num_workers > MAX_WORKERS){ num_workers = MAX_WORKERS; }
    omp_set_num_threads(num_workers);

    int *to_sort = malloc(num_elems * sizeof(int));
    int *tmp = malloc(num_elems * sizeof(int));

    // fill up array with randomized numbers
    int modulo = 99;
    for(int i = 0; i < num_elems; i++){
        to_sort[i] = rand()%modulo;
    }

    #ifdef DEBUG
        printf("{");
        for(int i = 0; i < num_elems; i++){
            printf("%d ", to_sort[i]);
        }
        printf("{\n");
    #endif
    
    double start_time = omp_get_wtime();
    #pragma omp parallell
    {
        #pragma omp single
        printf("calling quicksort\n");
        quicksort(to_sort, 0, num_elems - 1);
    }
    double end_time = omp_get_wtime();

    #ifdef DEBUG
        printf("{");
        for(int i = 0; i < num_elems; i++){
            printf("%d ", to_sort[i]);
        }
        printf("}\n");
    #endif
    assert(isSorted(to_sort, num_elems));
    printf("executed in %g seconds, matrix size of %d and %d threads\n", end_time - start_time, num_elems, num_workers);
}