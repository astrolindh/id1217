/*
Homework assignement 2, ID1217 VT22
Astrid Lindh
    build and compile with
        gcc -o open openmatrix.c -fopenmp

    task 1
    Generate a matrix of size m*m and populate it with random integers
    in parallel execution, sum up all of the elements to a total
    Also, find the/a global maximum and minimum values with their corresponding values within the matrix.
    Time the part of the program that executes in parallel
*/

#include <stdlib.h>
#include <stdio.h>
#include <omp.h>


#define MAXSIZE 10000           // max matrix size
#define STANDARDSIZE 10000        // matrix size, if not specified
#define MAXWORKERS 8            // max number of workers
#define STANDARDWORKERS 8       // number of workers, if not specified

double start_time, end_time;
int num_workers;
int size;
int matrix [MAXSIZE][MAXSIZE];
void *Worker(void *);           // placeholder procedure


// stores max or min values with index position
struct extreme_val{
    int value;
    int row;
    int col;
};

int main(int argc, char *argv[]){
    int m, n, total = 0;
    // attempt to read command line arguments, check that arguments are within bounds
    size = (argc > 1)? atoi(argv[1]) : STANDARDSIZE;
    if(size > MAXSIZE){size = STANDARDSIZE;}
    num_workers = (argc > 2)? atoi(argv[2]) : STANDARDWORKERS;
    if(num_workers > MAXWORKERS){num_workers = STANDARDWORKERS;}

    omp_set_num_threads(num_workers);

    // initialize matrix
    int modulo = 99;
    for(m = 0; m < size; m++){
        for(n = 0; n < size; n++){
            matrix[m][n] = rand()%modulo;
        }
    }

    #ifdef DEBUG
        printf("[");
        for(m = 0; m < size; m++){
            for(n = 0; n < size; n++){
                printf("%d, ", matrix[m][n]);
            }
            printf("\n");
        }
        printf("]\n");
    #endif
    

    // Data structures to hold found max, min values
    struct extreme_val maxs[num_workers];
    struct extreme_val mins[num_workers];

    struct extreme_val max = {-1, 0, 0};
    struct extreme_val min = {modulo, 0, 0};  

    for(int i = 0; i < num_workers; i++){
        maxs[i] = max;
        mins[i] = min;
    }

    int local_maxs[num_workers];
    int local_mins[num_workers];
    for(int i = 0; i < num_workers; i++){
        local_maxs[i] = 0;
        local_mins[i] = modulo;
    }
    // STARTING PARALLEl EXECUTION
    start_time = omp_get_wtime();
      
    #pragma omp parallell for reduction (+:total) private(n)
    for(m  = 0; m < size; m++){
        for(n = 0; n < size; n++){
            int a = matrix[m][n];
            total += a;
            int thread_num = omp_get_thread_num();
            if(local_maxs[thread_num] < a){
                local_maxs[thread_num] = a;
                struct extreme_val ext = {a, m, n};
                maxs[thread_num] = ext;
            }
            else if(local_mins[thread_num] > a){
                local_mins[thread_num] = a;
                struct extreme_val ext = {a, m, n};
                mins[thread_num] = ext;
            }
        }
    }
    // end pragma, implicit barrier

    end_time = omp_get_wtime();
    printf("the total is %d\n", total);
    
    // extract, max, min values
    int lowest = modulo;
    int highest = -1;
    int index_max, index_min;
    if(num_workers < 2)
    {
        index_min = 0;
        index_max = 0;
    }
    else{
        for(int i = 0; i < num_workers; i++){
            if(maxs[i].value > highest)
            {
                highest = maxs[i].value;
                index_max = i;
            }
            if(mins[i].value < lowest){
                lowest = mins[i].value;
                index_min;
            }
        }
    }

    min = mins[index_min];
    max = maxs[index_max];
    printf("min value %d found at [%d, %d]\n", min.value, min.row, min.col);
    printf("max value %d found at [%d, %d]\n", max.value, max.row, max.col);
    printf("executed in %g seconds, matrix size of %d and %d threads\n", end_time - start_time, size, num_workers);

    
}


