/**
 * MPI Program that performs simple sorting
 *
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"



double * merge_array(int n, double * a, int m, double * b);
void     merge_sort(int n, double * a);
void     swap (double * a, double * b);

int main (int argc, char *argv[])
{

	int rank, size;

	int n = 1000, i, j, k, x, q, l, shell, pair, *nr;
	double m = 10.0;
	double * scattered_array, * array;

	// Init + rank + size
	MPI_Init(&argc, &argv);
   	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   	MPI_Comm_size(MPI_COMM_WORLD, &size);


	if( rank == 0 )
	{

	   //initialise the array with random values, then scatter to all processors
       array = (double *) calloc( n, sizeof(double) );
	   srand( ((unsigned)time(NULL)+rank) );

	   for( i = 0; i < n; i++ )
	   {
	      array[i]=((double)rand()/RAND_MAX)*m;
	   }

	}


	// scatter array to scattered_array with double elements
	MPI_Scatter(array, n/size, MPI_INT, scattered_array, n/size, MPI_INT, 0, MPI_COMM_WORLD);
	// sort scattered_array
	merge_sort( n/size, scattered_array);


	// gather scattered_array to array with double elements
	MPI_Gather(scattered_array, n/size, MPI_INT, array, n, MPI_INT, 0, MPI_COMM_WORLD);


	if( rank == 0 )
	{

	  // merge the size chunks of array
		merge_sort(n, array);
	  // print array
		for(int i =0; i<n; i++){

			printf("%f ", array[i]);

		}

	}

	MPI_Finalize();

}

// function to merge the array a with n elements with the array b with m elements
// function returns the nerged array

double * merge_array(int n, double * a, int m, double * b){

   int i,j,k;
   double * c = (double *) calloc(n+m, sizeof(double));

   for(i=j=k=0;(i<n)&&(j<m);)

      if(a[i]<=b[j])c[k++]=a[i++];
      else c[k++]=b[j++];

   if(i==n)for(;j<m;)c[k++]=b[j++];
   else for(;i<n;)c[k++]=a[i++];

return c;
}

// function to merge sort the array a with n elements

void merge_sort(int n, double * a){

   double * c;
   int i;

   if (n<=1) return;

   if(n==2) {

      if(a[0]>a[1])swap(&a[0],&a[1]);
      return;
   }



   merge_sort(n/2,a);merge_sort(n-n/2,a+n/2);

   c=merge_array(n/2,a,n-n/2,a+n/2);

   for(i=0;i<n;i++)a[i]=c[i];

return;
}


// swap two doubles
void swap (double * a, double * b){

   double temp;

   temp=*a;*a=*b;*b=temp;

}
