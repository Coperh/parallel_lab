
#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

char * rpsMoves = "RPS";
 
int testGame(char move1, char move2);



int main(int argc, char ** argv){
    int rank, size;

       // init + rank + size
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

        // generate a random move
    srand(rank*((unsigned int)time(NULL)));
    int move = rand()%3, move1 = -1;
    MPI_Status status;
    int tag1 = 1, tag2 = 2, winner = -1;;

        // ping pong move to move1
    if(rank == 0){
        MPI_Send(&move,  1, MPI_INT, 1, tag1, MPI_COMM_WORLD);
        MPI_Recv(&move1, 1, MPI_INT, 1, tag2, MPI_COMM_WORLD, &status);
 
        winner = testGame(rpsMoves[move], rpsMoves[move1]);
    }else{
        MPI_Recv(&move1, 1, MPI_INT, 0, tag1, MPI_COMM_WORLD, &status);
        MPI_Send(&move,  1, MPI_INT, 0, tag2, MPI_COMM_WORLD);
 
        winner = testGame(rpsMoves[move1], rpsMoves[move]);
    }
    

    printf("Player %d finds the winner %d\n", rank, winner);

    MPI_Finalize();
    return 0;
}


int testGame(char move1, char move2){
    if (move1 == move2)return 2;
    else if((move1=='R'&move1=='S') || 
    (move1=='S'&move1=='P') || (move1=='P'&move1=='R') )return 0;
    else return 1;

}