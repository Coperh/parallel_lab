
#include "mpi.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>


// Declaration of some functions to work with

int ** alloc_matrix(int n, int m);
void init_matrix(int n, int m, int ** a,int p);
int ** prod_matrix(int n, int l, int m, int ** a, int ** b);
void print_matrix(int n, int m, int ** a);
void extract_matrix(int na, int ma, int ** a, int nb, int mb, int ** b, int row, int col);
void implant_matrix(int na, int ma, int ** a, int nb, int mb, int ** b, int row, int col);


int main(int argc, char ** argv){


	  int n=12, p,i ,j, rank, size, senderRank,step,recvRank, leftRank, rightRank, upRank, downRank;


	  // matrix declarations
	  int **a, **local_a, **b, **local_b, **c, **local_c, **tmp_c;

	  // MPI variables
	  MPI_Comm grid;

	  MPI_Status status;

	  int dims[2], periods[2], coords[2], reorder=0, senderCoords[2],recvCoords[2];
	  int leftCoords[2],rightCoords[2],upCoords[2],downCoords[2];

	  int tag1=1, tag2=2;

	  double time;

	  // initialise the mpi world and get basic infomation
	  MPI_Init(&argc, &argv);
   	  MPI_Comm_size(MPI_COMM_WORLD, &size);
	  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	  // test if possible to make the square grid
	  p=(int)sqrt(size);
	  if(p*p!=size){

		  printf("square grid not posible with %d procs\n", size);
		  printf("processor %d stopped", rank);

	      MPI_Finalize();

		  return 0;

	  }

	  // alocate the matrix a
  	  if(rank==0){

	  		  // intialise the matrix(ces) a
	  		  a=alloc_matrix(n,n); init_matrix(n,n,a,n/p);
              b=alloc_matrix(n,n); init_matrix(n,n,b,n/p);
          

	  }


	// create the grid communicator
	  dims[0] = dims[1] = p;
	  periods[0] = periods[1] = 1;
	  MPI_Cart_create(MPI_COMM_WORLD,2,dims,periods,reorder,&grid);


	// find the coordinates of rank in grid as well as the rank of left, right, up and down.
	  MPI_Comm_rank(grid,&rank);
	  MPI_Cart_coords(grid,rank,2,coords);

	  leftCoords[0] = (coords[0] - 1)%p; leftCoords[1] = coords[1];
	  rightCoords[0] = (coords[0] + 1)%p; rightCoords[1] = coords[1];
	  upCoords[0] = coords[0]; upCoords[1] = (coords[1]-1)%p;
	  downCoords[0] = coords[0]; downCoords[1] = (coords[1]+1)%p;

	  MPI_Cart_rank(grid, leftCoords, &leftRank);
	  MPI_Cart_rank(grid, rightCoords, &rightRank);
	  MPI_Cart_rank(grid, upCoords, &upRank);
	  MPI_Cart_rank(grid, downCoords, &downRank);
    
    
    time = MPI_Wtime();

    // -------------------------- PARTITION MATRIX A ----------------------------------
    
	// allocate the local matrix(ces)
	  local_a=alloc_matrix(n/p,n/p);

	// if processor 0 extract the submatrix i,j from a and sent it to the processor i,j
	if (rank==0)
	{
		  for(i =0 ;i < p ; i++) for (j = 0; j<p; j++){
              //extract the block i,j from a
              extract_matrix(n, n, a, n/p, n/p, local_a, i*n/p, ((j-i+p)%p)*n/p );
              //send the buffer b to processor i,j
              recvCoords[0] = i;
              recvCoords[1] = j;
              MPI_Cart_rank(grid,recvCoords,&recvRank);
              MPI_Send(&local_a[0][0], n*n/size, MPI_INT, recvRank , 1, MPI_COMM_WORLD);

	  }
	}

	MPI_Recv(&local_a[0][0], n*n/size, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
    
    // -------------------------- PARTITION MATRIX B ----------------------------------
    
    // allocate the local matrix(ces)
      local_b=alloc_matrix(n/p,n/p);

    // if processor 0 extract the submatrix i,j from a and sent it to the processor i,j
    if (rank==0)
    {
          for(i =0 ;i < p ; i++) for (j = 0; j<p; j++){
              //extract the block i,j from a
              extract_matrix(n, n, b, n/p, n/p, local_b, ((i-j+p)%p)*n/p, j*n/p );
              //send the buffer b to processor i,j
              recvCoords[0] = i;
              recvCoords[1] = j;
              MPI_Cart_rank(grid,recvCoords,&recvRank);
              MPI_Send(&local_b[0][0], n*n/size, MPI_INT, recvRank , 1, MPI_COMM_WORLD);

      }
    }

    MPI_Recv(&local_b[0][0], n*n/size, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
    
    // --------------------------- CANNON MULTIPLICATION -------------------------------
    local_c = alloc_matrix(n/p, n/p);
    c       = alloc_matrix(n,n);
    tmp_c   = alloc_matrix(n/p, n/p);
    
    for(int step = 0;step<p;step++){
        
        // multiply local_a * local_b and accumulate the result to local_c
        tmp_c = prod_matrix(n/p, n/p, n/p, local_a, local_b);
        for(int i=0;i<n/p;i++)for(int j=0;j<n/p;j++)local_c[i][j] += tmp_c[i][j];
        
        // roll local_a to left
        MPI_Send(local_a[0], n*n/size, MPI_INT, leftRank, tag1, MPI_COMM_WORLD);
        MPI_Recv(local_a[0], n*n/size, MPI_INT, rightRank, tag1, MPI_COMM_WORLD, &status);
        // we can use MPI_Sendrecv_replace
        
        // roll local_b up
        MPI_Send(local_b[0], n*n/size, MPI_INT, upRank,   tag2, MPI_COMM_WORLD);
        MPI_Recv(local_b[0], n*n/size, MPI_INT, downRank, tag2, MPI_COMM_WORLD, &status);

        
    }

    // ------------------------ collect local_c block from processors -------------------
    MPI_Send(local_c[0], n*n/size, MPI_INT, 0, tag1, MPI_COMM_WORLD);
    
    if(rank==0){
        
        for(int i=0;i<p;i++)for(int j=0;j<p;j++){
            
            senderCoords[0]=i;senderCoords[1]=j;
            MPI_Cart_rank(grid, senderCoords, &senderRank);
            
            MPI_Recv(local_c[0], n*n/size, MPI_INT, senderRank, tag1, MPI_COMM_WORLD, &status);
            
            implant_matrix(n, n, c, n/p, n/p, local_c, i*n/p, j*n/p);
            
        }
    }
    
    time = MPI_Wtime()-time;
    
    printf("cannon: processor %d finds %lf seconds\n", rank, time);
    

	MPI_Finalize();


}



/*

the function alloc_matrix allocates dinamically a matrix with n row and m colums

the arguments are

  n ==> number of rows
  m ==> number of columns
  the function returns the
matrix pointer


*/

int ** alloc_matrix(int n, int m){

        int i, j, **a, *aa;

        aa=(int *) calloc(n*m, sizeof(int));
        a=(int **) calloc(n, sizeof(int*));

        for(i=0;i<n;i++)a[i]=aa+i*m;

        for(i=0;i<n;i++)for(j=0;j<m;j++)a[i][j]=0;

return a;
}


/*

the function init_matrix initialises the matrix a with random values

the arguments are

  n ==> number of rows
  m ==> number of columns
  a ==> matrix


*/


void init_matrix(int n, int m, int ** a, int p){

        int i, j;

        //for(i=0;i<n;i++)for(j=0;j<m;j++)a[i][j]=rand()%100;
	 for(i=0;i<n;i++)for(j=0;j<m;j++)a[i][j]=i/p+j/p;

return;
}


/*

the function prod_matrix is to multiply two matrices:

a with n rows and l colums and b with with l rows and m colums

the arguments are

  n ==> number of rows of a
  l ==> number of colums of a (and rows of b)
  m ==> number of columns of b
  a ==> matrix 1
  b ==> matrix 2

the function returns the matrix pointer c


*/



int ** prod_matrix(int n, int l, int m, int ** a, int ** b){

        int i,j,k,** c;

        c=alloc_matrix(n,m);

        for(i=0;i<n;i++)for(j=0;j<m;j++){

                c[i][j]=0;

                for(k=0;k<l;k++)c[i][j]=c[i][j]+a[i][k]*b[k][j];

        }


return c;

}

/*

the function init_matrix prints the matrix a

the arguments are

  n ==> number of rows
  m ==> number of columns
  a ==> matrix


*/


void print_matrix(int n, int m, int ** a){

        int i,j;

        for(i=0;i<n;i++){

			for(j=0;j<m;j++)printf(" %d ",a[i][j]);
			printf("\n");

		}

return;
}


/*

the function extract_matrix extracts from the matrix a with na rows and ma columns

a submatrix b with nb rows and mb columns starting from the element row and col.

the arguments are

  na ==> number of rows of a
  ma ==> number of columns of a
  a ==> matrix
  nb ==> number of rows of b
  mb ==> number of columns of b
  b ==> extracted matrix
  row, col ==> where to extract from

*/


void extract_matrix(int na, int ma, int ** a, int nb, int mb, int ** b, int row, int col){

        int i,j;

		if(na<row+nb || na<col+mb){

			printf("Impossible to extract");
			return;

		}

        for(i=0;i<nb;i++)
			for(j=0;j<mb;j++)
				b[i][j]=a[row+i][col+j];



}


/*

the function implant_matrix is the opposite to the function extract_matrix


  na ==> number of rows of a
  ma ==> number of columns of a
  a ==> matrix
  nb ==> number of rows of b
  mb ==> number of columns of b
  b ==> extracted matrix
  row, col ==> where to extract from

*/

void implant_matrix(int na, int ma, int ** a, int nb, int mb, int ** b, int row, int col){

        int i,j;

		if(na<row+nb || na<col+mb){

			printf("Impossible to extract");
			return;

		}

        for(i=0;i<nb;i++)
			for(j=0;j<mb;j++)
				a[row+i][col+j]=b[i][j];



}
