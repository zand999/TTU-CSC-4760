//*****************************************************//1
// Spring 2023 - CSC 4760/5760 - Assignment 2 Base Code 
// This program simulates an archer shooting a target. 
// Each hit rewards 3 points, however the archer's accuracy is random.
// INPUT: Number of shots taken.  
// OUTPUT: Average score per shot.
// 
//TO DO:*****************************************************************//
// 1. Read number of samples and thread count from argv
// 2. Generate threads and seperate workload.                    
// 3. Add timers for the overall program and each of the threads.           //
// 4. Print the execution time of each thread
// 5. Print the overall execution time. //  
//**************************************************************************//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

int main(int argc, char* argv[])
{
	//declare variables
	int hit_total=0 ,samplecount_total=0;
	double result = 0.0; 
	
	//seed random number generator from system clock
	srand( (unsigned)time( NULL ) );

	double truntimes = omp_get_wtime();
	
	#pragma omp parallel shared(hit_total, samplecount_total)
	{
		double stime = omp_get_wtime(),etime;
		double x, y, check;
		int hit = 0,samplecount = omp_get_thread_num() * 1000;
		printf("%d theadstart samplecount: %d\n",omp_get_thread_num(), samplecount);
		#pragma omp critical
			samplecount_total += samplecount;
		
		for(int i=0; i<samplecount; i++){
			//generate sample
			x = (double)rand()/RAND_MAX;
			y = (double)rand()/RAND_MAX;
		
			//calculate if hit or miss
			check = x*x + y*y;
			if(check <= 1){
			hit++;
			}
		}
		#pragma omp critical
			hit_total += hit;
		etime = omp_get_wtime();

		printf("%d theadend, Runtime: %f\n",omp_get_thread_num(), etime - stime);
		
	}
	
	
	//calculate and return result
	result = 3 * (double) hit_total /samplecount_total;
	printf("Result: %.5f\n", result);
	printf("Runtime: %f\n", omp_get_wtime() - truntimes);
	
   return 0; 
}
