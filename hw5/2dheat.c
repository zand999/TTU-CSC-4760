#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

const int MAX_STRING = 100;

//struct for heating point data
typedef struct{

    int row;
    int col;
    double temp;

}Heater;

typedef struct{

    int row;
    int col;

}Cord;



void procecssStagger(int id){
    //usefull process scatter 
    for(long i = 0; i < (long)id * 30000000; i++){

    }
}

//helper function to calculate location in 1d array that is treated as 2d
int elementloc(int width, int row, int col){

    return (width * row) + col;
}
//helper function to calulate location in internal 2d array
/*
int array[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};
    for(int i = 0; i < 2; i++){
        for(int j = 0; j < 4; j++){
            printf("%d,", array[internalelementloc(6,i,j)]);
        }
        printf("\n");
    }
*/
int internalelementloc(int width, int row, int col){
    
    return width + (width * row) + 1+ col;
}



//function to import the heater location data from file
Heater *heaterlist(char filename[], int *size){

    //printf("%d\n", *size);
    FILE *fptr;
    fptr = fopen(filename, "r");
    if (fptr == NULL) {
        printf("no such file.");
        return 0;
    }else{
        fscanf(fptr, "%d", size);
        //printf("%d\n", *size);
        Heater *harray = calloc(*size, sizeof(Heater));
        
        //printf("Heater:%d\nharray:%d\n",sizeof(Heater),sizeof(harray));
        for(int i = 0; i < *size; i++){
            
            //fscanf(fptr, "%d %d %f", &row, &col, &temp);
            fscanf(fptr, "%d %d %lf", &harray[i].row, &harray[i].col, &harray[i].temp);
            //printf("%d %d %f\n",harray[i].row, harray[i].col, harray[i].temp );
        }

        fclose(fptr);

        return harray;
    }
    fclose(fptr);
    return NULL;
}



//function to output data  to file
int gridToCSV(char filename[], double grid[], int row_l, int col_h){


    FILE *fptr;
    fptr = fopen(filename, "w+");
    if (fptr == NULL) {
        printf("no such file.");
        return 0;
    }else{

        for(int i = 0; i < col_h; i++){

            for(int j = 0; j < row_l - 1; j++){
                
                fprintf(fptr, "%lf,", grid[elementloc(col_h,i,j)]);

            }
            fprintf(fptr, "%lf\n", grid[elementloc(col_h,i,row_l - 1)]);
        }
    }
    fclose(fptr);
    return 1;
}



//Main function: args: <rows> <cols> <k> <boundary_temp> <time_steps> <fixed_temps_file> [outfile]
int main(int argc, char *argv[] ){

    //argument variable declriation
    int rows = atoi(argv[1]), cols = atoi(argv[2]);
    double k = atof(argv[3]), bound_temp = atof(argv[4]);
    int time_steps = atoi(argv[5]);
    double start, end;
    //mis var
    int heaternum = 0;
    //int bigArraySize = (rows+2)*(cols+2);
    
    //array of heater locations
    Heater *heaters = heaterlist(argv[6],&heaternum );


    MPI_Init(NULL,NULL);

    //get num of processes
    int num_processes;
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
    

    // Get the rank of the process
    int process_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

    //calculate runtime
    start = MPI_Wtime();

    //create process chunks
    int chunksize = (rows)/ num_processes;
    if(process_rank < ((rows) % (num_processes))){
        chunksize++;
    }
    int chunkln = (chunksize + 2) * (cols +2);
    double *array1 = calloc(chunkln,sizeof(double));
    double *array2 = calloc(chunkln,sizeof(double));
    double *temparray;

    //init first chunk
    //why TF this doesnt work IDK
    /*for(int i; i < chunkln-1; i ++){
        
        //array1[i] = (double)i + process_rank;
        array2[i] = bound_temp;
        array1[i] = bound_temp;
    }*/
    for(int i = 0; i < chunksize+2; i++){

        for(int j = 0; j < rows + 2; j++){
            array2[(i)*(rows+2) +(j)] = bound_temp;
            array1[(i)*(rows+2) +(j)] = bound_temp;
            //printf("%1.2lf,", array1[(i)*(rows+2) +(j)]);
        }
        
    }
    
    //calculate rows offset
    int priorrows = 0;
    for(int i = 0; i < process_rank; i++){
        //process_rank < ((rows+2) % (num_processes)) 
        priorrows += (((rows)/ num_processes) + (i < ((rows) % (num_processes))? 1 : 0));
    }

    

    //display starting enviroment cause why not
    if(process_rank == 0){ 

        //initlizte variables
        printf("process num:%d\n",num_processes );
        printf("ARGS:<%d,%d,%f,%f,%d,%s,%s>\n", rows, cols, k, bound_temp, time_steps,argv[6], argv[7]);
        printf("Heater Data:\n%d\n", heaternum);
        for(int i = 0; i < heaternum; i++){
            
            printf("%d %d %f\n",heaters[i].row, heaters[i].col, heaters[i].temp );
        }
        MPI_Barrier(MPI_COMM_WORLD);
        printf("Process %d chunksize: %d chunkln: %d priorrows: %d\n", process_rank, chunksize, chunkln, priorrows);
    }else{
        MPI_Barrier(MPI_COMM_WORLD);
        printf("Process %d chunksize: %d chunkln: %d priorrows: %d\n", process_rank, chunksize, chunkln, priorrows);
    }
    
    
    
    
    //calculate new heater cords per process
    for(int i = 0; i < heaternum; i++){

        //calculate heater offset for thread    
        heaters[i].row = heaters[i].row - priorrows;
        heaters[i].col += 1;

        //place heaters in threads
        if(heaters[i].row >= 0 && heaters[i].row < chunksize){

            //width + (width * row) + 1+ col;
            //set initial heater locations
            array1[(rows+2) + ( (rows+2) * heaters[i].row)  + heaters[i].col] = heaters[i].temp;

        }

        

    }
    
    //first halo exchange for ghost row to transfer heat locations
    if(process_rank ==0 && num_processes > 1){ 
        //send
        /*for(int j = 0; j < cols+2; j++){
            printf("%dS:%1.2lf,",j, array1[((cols+2) * (chunksize)) + j] );
        }
        printf("\n");*/

        //send top
        MPI_Send(array1 + (((cols+2) * (chunksize))),cols+2, MPI_DOUBLE, 1,0,MPI_COMM_WORLD);
        //recieve top
        MPI_Recv(array1 + (((cols+2) * (chunksize+1))),cols+2, MPI_DOUBLE, process_rank + 1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        
    }else if(process_rank == num_processes - 1 && process_rank !=0){

        
            
        //recieve array bottom
        MPI_Recv(array1,cols+2, MPI_DOUBLE, process_rank - 1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

        //send array bottom
        MPI_Send(array1 + (cols+2),cols+2, MPI_DOUBLE, process_rank - 1,0,MPI_COMM_WORLD);
        
    }else if(num_processes > 1){
        
        //recieve bottom
        //if(process_rank == 1){
        MPI_Recv(array1,cols+2, MPI_DOUBLE, process_rank - 1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        /*for(int j = 0; j < cols+2; j++){
            printf("%dR:%1.2lf,",process_rank ,array1[j]);
        }
        printf("\n");*/
        //}
        //send top array
        MPI_Send(array1 + (((cols+2) * (chunksize))),cols+2, MPI_DOUBLE, process_rank + 1,0,MPI_COMM_WORLD);

        //recieve top
        MPI_Recv(array1 + (((cols+2) * (chunksize+1))),cols+2, MPI_DOUBLE, process_rank + 1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        //send array bottom
        MPI_Send(array1 + (cols+2),cols+2, MPI_DOUBLE, process_rank - 1,0,MPI_COMM_WORLD);

        //

    }
    
    //do iterations
    double tempavg;
    for(int iter = 0; iter < time_steps;iter++){

        //COMPUTE
        for(int i = 1; i < chunksize+1; i++){

            for(int j = 1; j < rows + 1; j++){
                //array2
                //do the stecil math
                //TN = k*(TL+T+TR+L+R+BL+B+BR)/8 + (1-k)*TO
                tempavg = (array1[((i-1)*(rows+2) +(j-1))] +array1[((i-1)*(rows+2) +(j))]+array1[((i-1)*(rows+2) +(j+1))]+array1[((i)*(rows+2) +(j-1))]+array1[((i)*(rows+2) +(j+1))]+array1[((i+1)*(rows+2) +(j-1))]+array1[((i+1)*(rows+2) +(j))]+array1[((i+1)*(rows+2) +(j+1))])/8;
                array2[(i * (rows+2) +j)] = k *  tempavg + (1-k) * array1[(i * (rows+2) +j)];

            }
        }
        //halo echange
        if(process_rank ==0 && num_processes > 1){ //first process starts
            //send
            /*for(int j = 0; j < cols+2; j++){
                printf("%dS:%1.2lf,",j, array1[((cols+2) * (chunksize)) + j] );
            }
            printf("\n");*/

            //send top
            MPI_Send(array2 + (((cols+2) * (chunksize))),cols+2, MPI_DOUBLE, 1,0,MPI_COMM_WORLD);
            //recieve top
            MPI_Recv(array2 + (((cols+2) * (chunksize+1))),cols+2, MPI_DOUBLE, process_rank + 1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            
        }else if(process_rank == num_processes - 1 && process_rank !=0){//do stuff for last process

                
            //recieve array bottom
            MPI_Recv(array2,cols+2, MPI_DOUBLE, process_rank - 1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

            //send array bottom
            MPI_Send(array2 + (cols+2),cols+2, MPI_DOUBLE, process_rank - 1,0,MPI_COMM_WORLD);
            
        }else if(num_processes > 1){
            
            //recieve bottom
            //if(process_rank == 1){
            MPI_Recv(array2,cols+2, MPI_DOUBLE, process_rank - 1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            /*for(int j = 0; j < cols+2; j++){
                printf("%dR:%1.2lf,",process_rank ,array1[j]);
            }
            printf("\n");*/
            //}
            //send top array
            MPI_Send(array2 + (((cols+2) * (chunksize))),cols+2, MPI_DOUBLE, process_rank + 1,0,MPI_COMM_WORLD);

            //recieve top
            MPI_Recv(array2 + (((cols+2) * (chunksize+1))),cols+2, MPI_DOUBLE, process_rank + 1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            //send array bottom
            MPI_Send(array2 + (cols+2),cols+2, MPI_DOUBLE, process_rank - 1,0,MPI_COMM_WORLD);

            //

        }
        //update heat locations
        for(int i = 0; i < heaternum; i++){//do stuff for middle processes
            if(heaters[i].row >= 0 && heaters[i].row < chunksize){
                //set initial heater locations
                array2[(rows+2) + ( (rows+2) * heaters[i].row)  + heaters[i].col] = heaters[i].temp;

            }
        }
        
        //TO DA SWAP
        temparray = array2;
        array2 = array1;
        array1 = temparray;

    }
    
   

    //stich together partitions
    //MPI_Barrier(MPI_COMM_WORLD);
    //procecssStagger(process_rank);
    if(process_rank == 0){ 
    
        //alocate space for final grid in thread 0 
        double *finalgrid = calloc(rows*cols,sizeof(double));
        //alocate space for partition buffer of largest size
        double *chunkbuff = calloc(chunkln,sizeof(double));


        //move partition data from other threads to final array
        //cant be done with copy as array has ghost data on all sides, also mem safty
        if(num_processes > 1){
            
           
            for(int i = 1,threadprows = 0,threadsize = 0; i < num_processes; i++){

                //get partition data
                MPI_Recv(chunkbuff, chunkln, MPI_DOUBLE, i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                
                //calculate thread partition data size information
                threadprows = 0;
                for(int j = 0; j< i; j++){
                    threadprows += (((rows)/ num_processes) + (j < ((rows) % (num_processes))? 1 : 0));
                }
                threadsize = (rows)/ num_processes;
                if(i < ((rows) % (num_processes))){
                    threadsize++;
                }

                //move the data from partition
                for(int i = 0; i < threadsize; i++){
                    for(int j = 0; j < cols ; j++){
                        //!!!!!!!!!Convert to temp array later
                        //printf("%d,", (i*cols) + j);
                        //finalgrid[ (i*cols) + (priorrows * cols) + j] = chunkbuff[internalelementloc(cols+2,i,j)];
                        //printf("(%d)", (i*cols) + (threadprows * cols) + j);
                        //finalgrid[ (i*cols) + (threadprows * cols) + j] = 50;
                        finalgrid[ (i*cols) + (threadprows * cols) + j] = chunkbuff[internalelementloc(cols+2,i,j)];
                    }
            
                }

            }
                


        }
        
        //move array data from other thread0 to final array
        //cant be done with copy as array has ghost data on all sides, also mem safty
        for(int i = 0; i < chunksize; i++){
            for(int j = 0; j < cols ; j++){
                finalgrid[ (i*cols) + j] = array1[internalelementloc(cols+2,i,j)];
            }
            
        }

        
        //HANDY DANDY PRINT HELP
        printf("\n");
        int printgrid = 0;
        if(printgrid == 1){
            for(int i = 0; i < rows; i++){
                for(int j = 0; j < cols; j++){
                    printf("%1.2lf,", finalgrid[( cols* i) + j]);
                }
                printf("\n");
            }
        }
        if(printgrid == 2){
            for(int i = 0; i < chunksize+2; i++){
                for(int j = 0; j < cols+2 ; j++){
                    
                    printf("%1.2lf,", array1[( (cols+2) * i) + j]);
                }
                printf("\n");
            }
            
        }

        //do csv output
        gridToCSV(argv[7], finalgrid, rows, cols);

        
     
    }else{

        //send to process 0
        MPI_Send(array1,chunkln, MPI_DOUBLE, 0,0,MPI_COMM_WORLD);

    }

    //MPI_Barrier(MPI_COMM_WORLD);
    end = MPI_Wtime();
    if(process_rank == 0){ 
        printf("\nDONE Output:%s\n", argv[7]);
        printf("Runtime: %lf\n", end-start);
    }

    MPI_Finalize();
    
    return 0;

    

}


    