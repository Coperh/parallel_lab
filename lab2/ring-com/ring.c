

#include	<stdio.h>
#include	<mpi.h>

#define tag 1

int main (int argc, char *argv[]){

	int ierror, rank, size;  int right, left; int ibuff, obuff, sum, i;

	MPI_Status  recv_status; 

	// init + rank + size 
	MPI_Init(&argc, &argv);
    	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    	MPI_Comm_size(MPI_COMM_WORLD, &size);

    	// find the righ and left processor
    	right = rank + 1; if (right == size) right = 0;
    	left = rank - 1;    if (left == -1) left = size-1;

    	sum = 0;  obuff = rank;

    	// communication around the ring
    	for( i = 0; i < size; i++) {
    	    MPI_Send(&obuff, 1, MPI_INT, right, tag, MPI_COMM_WORLD);
    	    MPI_Recv(&ibuff, 1, MPI_INT, left, tag, MPI_COMM_WORLD, &recv_status);
 
     	   sum = sum + ibuff; obuff = ibuff;
    	}
    	
    	printf ("\t Processor %d: \t Sum = %d\n", rank, sum);

    MPI_Finalize();
}
