#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define tag 1


char * rpsMoves = "RPS";
 
int testGame(char move1, char move2);


int main(int argc, char ** argv){
    int ierror, rank, size;  int right, left; int ibuff, obuff, sum, i;
 
    // init + rank + size
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
 
    // generate a random move
    srand(rank*((unsigned int)time(NULL)));
    int move = rand()%3, move1 = -1;
    MPI_Status  recv_status;


    // ring communication
    // find the righ and left processor
    right = rank + 1; if (right == size) right = 0;
    left  = rank - 1;  if (left == -1) left = size-1;
 
    obuff = move;
    int * moves = (int *)calloc(size, sizeof(int));
 
    // communication around the ring
    for( int i = 0; i < size; i++) {
        int pos;

        if (rank - i < 0) pos = (size) + (rank-i);
        else pos = rank-i; 
        moves[pos] = obuff;
        
        MPI_Send(&obuff, 1, MPI_INT, right, tag, MPI_COMM_WORLD);
        MPI_Recv(&ibuff, 1, MPI_INT, left, tag, MPI_COMM_WORLD, &recv_status);
 
        obuff = ibuff;
    }

    // how to test the winner of the moves
 
    for(int i = 0; i <size; i++){

        int j  = i+1 < size? i+1 :0;
        int move1 = moves[i];

        int move2 = moves[j];

        int result = testGame(rpsMoves[move1], rpsMoves[move2]);
        
        int winner = i;
        int loser = j;

        if(result == 2){
          printf("Player %d finds draw between Player %d and Player %d: (%c, %c)\n",
         rank, winner, loser, rpsMoves[move1], rpsMoves[move2]);      
        } 
        else{
            if( result == 1){
                winner = j; loser = i;
            }
            printf("Player %d finds the winner Player %d against Player %d: (%c, %c)\n",
            rank, winner, loser, rpsMoves[move1], rpsMoves[move2]);
        }
    }

    
    MPI_Finalize();
 
    return 0;
}



int testGame(char move1, char move2){
    if (move1 == move2)return 2;
    else if((move1=='R'&move1=='S') || 
    (move1=='S'&move1=='P') || (move1=='P'&move1=='R') )return 0;
    else return 1;

}