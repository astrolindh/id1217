// baserat på given övningsuppgift
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define MAX_THREADS 32

static long num_steps = 1000000;
double step;

int main(){
    int i, j;
    double pi, full_sum = 0.0;
    volatile double sum[MAX_THREADS];
    step = 1.0 / (double) num_steps;

    for(j = 1; j < MAX_THREADS; j = j*2){
        omp_set_num_threads(j);
        full_sum = 0.0;
        double start_time = omp_get_wtime();

        #pragma omp parallel
        {
            int i;
            int id = omp_get_thread_num();
            int numthreads = omp_get_num_threads();
            double x;
            sum[id] = 0.0;
            if(id == 0){ printf("num_threads är %d\n", numthreads); }
            for(i = id; i < num_steps; i += numthreads){
                x = (i + 0.5) * step;
                sum[id] = sum[id] + 4.0/(1.0 + x*x);
            }
        }
        // pragma end
        for(full_sum = 0.0, i = 0; i < j; i++){ full_sum += sum[i]; }
        pi = step * full_sum;
        double run_time = omp_get_wtime() - start_time;
        printf("approximated PI to %f,      in %f seconds, %d threads\n", pi, run_time, j);
    }
}