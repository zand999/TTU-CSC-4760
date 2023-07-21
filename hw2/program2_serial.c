//*****************************************************//
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

int main(int argc, char* argv[])
{
	//declare variables
	int hit=0, sample_count=1000000;
	double x, y, check, result = 0.0; 
	
	//seed random number generator from system clock
	srand( (unsigned)time( NULL ) );
	

	for(int i=0; i<sample_count; i++)
	{
		//generate sample
		x = (double)rand()/RAND_MAX;
		y = (double)rand()/RAND_MAX;
	
		//calculate if hit or miss
		check = x*x + y*y;
        if(check <= 1){
		   hit++;
		}
	}
	
	//calculate and return result
	result = 3 * (double)hit/sample_count;
	printf("%.5f", result);
	
   return 0; 
}