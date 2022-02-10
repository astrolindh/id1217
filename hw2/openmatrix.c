/*

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


int main(int argc, char *argv[]){
    int m, n, total = 0;
    // attempt to read command line arguments, check that arguments are within bounds
    size = (argc > 1)? atoi(argv[1]) : STANDARDSIZE;
    if(size > MAXSIZE){size = STANDARDSIZE;}
    num_workers = (argc > 2)? atoi(argv[2]) : STANDARDWORKERS;
    if(num_workers > MAXWORKERS){num_workers = STANDARDWORKERS;}

    omp_set_num_threads(num_workers);

    // initialize matrix

    for(m = 0; m < size; m++){
        for(n = 0; n < size; n++){
            matrix[m][n] = rand()%99;
        }
    }

    start_time = omp_get_wtime();
    #pragma omp parallell for reduction (+:total) private(j)
    for(m  = 0; m < size; m++){
        for(n = 0; n < size; n++){
            total +=matrix[m][n];
        }
    }
    // end pragma, implicit barrier

    end_time = omp_get_wtime();
    printf("the total is %d\n", total);
    printf("executed in %g seconds, matrix size of %d and %d threads\n", end_time - start_time, size, num_workers);
    
}


