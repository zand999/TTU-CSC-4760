#include <stdio.h>
#include <string.h>
#include <mpi.h>

const int MAX_STRING = 100;
int main(int argc, char *argv[] ){

    

    //printf("String:%s",argv[1]);
    char msg[MAX_STRING];
    sprintf(msg,"%s:",argv[1]);

    MPI_Init(NULL,NULL);

    int num_processes;
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);



    // Get the rank of the process
    int process_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

    if(process_rank ==0){ 

        sprintf(msg, "%s: %i", msg,process_rank );
        printf("Sending \"%s\" to: %i\n", msg,1);
        MPI_Send(msg,MAX_STRING, MPI_CHAR, 1,0,MPI_COMM_WORLD);
        MPI_Recv(msg, MAX_STRING, MPI_CHAR, num_processes-1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        //sprintf(msg, "%s->%i", msg,process_rank );
        printf("Rank 0 Recived: \"%s\" DONE\n", msg);
    }else{

        MPI_Recv(msg, MAX_STRING, MPI_CHAR, process_rank -1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        //printf("Recived: \"%s\"",msg)
        sprintf(msg, "%s->%i", msg,process_rank );
        //printf("Sending: \"%s\" to: %i\n", msg,(process_rank+1) % num_processes);
        MPI_Send(msg, MAX_STRING, MPI_CHAR, (process_rank+1) % num_processes,0,MPI_COMM_WORLD);

    }


    MPI_Finalize();
    
    return 0;

    

}