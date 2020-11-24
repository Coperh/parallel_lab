/***************************************************************


 MPI program to compute an array operation

****************************************************************/



#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "mpi.h"

int sum(int array[], int n, int size, int rank);
int product(int array[], int n, int size, int rank);

int max(int array[], int n, int size, int rank);
int min(int array[], int n, int size, int rank);

int main(int argc, char * argv []) {


	int * array;
	int n = 10000000; int n1;
	int size, rank;

	int result;

	// Init + rank + size
	MPI_Init (&argc,&argv);
	MPI_Comm_size (MPI_COMM_WORLD, &size);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);



	// 	populate the array a if processor 0
	// 	serial code
	if (rank == 0) {

	   // Processor 0 is the only one to know array
	   array = (int *)calloc(n, sizeof(int));
	   for(int i=0;i<n;i++)array[i]=1; // you can initialise with a random number


	}

	result = sum(array, n, size, rank);

	if(rank==0){

		// Write final_sum, final_prod, final_max and final_min

	   printf("the final sum is %d\n", result);

	}

	result = product(array, n, size, rank);

	if(rank==0){
	   printf("the final product is %d\n", result);
	}
	result = max(array, n, size, rank);

	if(rank==0){
	   printf("the final maximum is %d\n", result);
	}
	result = min(array, n, size, rank);

	if(rank==0){
	   printf("the final minimum is %d\n", result);
	}



	MPI_Finalize ();

}


int sum(int array[], int n, int size, int rank){

	int sum = 0, final_sum = 0;

	int * scattered_array;

	scattered_array=(int *)calloc(n/size, sizeof(int));



	// scatter the array onto processors
	// local array is scattered_array
	MPI_Scatter(array, n/size, MPI_INT, scattered_array, n/size, MPI_INT, 0, MPI_COMM_WORLD);


	// calculate sum, prod, max and min of scattered_array
	for(int i=0;i<n/size;i++)
	{
		sum+=scattered_array[i];
	}

	// reduce sum, prod, max, min to final_sum, final_prod, final_max and final_min
	MPI_Reduce(&sum,&final_sum,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);

	
	return final_sum;
}


int product(int array[], int n, int size, int rank){

	int product = 1, final_product = 1;

	int * scattered_array;

	scattered_array=(int *)calloc(n/size, sizeof(int));



	// scatter the array onto processors
	// local array is scattered_array
	MPI_Scatter(array, n/size, MPI_INT, scattered_array, n/size, MPI_INT, 0, MPI_COMM_WORLD);


	// calculate sum, prod, max and min of scattered_array
	for(int i=0;i<n/size;i++)
	{
		product*=scattered_array[i];
	}

	// reduce sum, prod, max, min to final_sum, final_prod, final_max and final_min
	MPI_Reduce(&product,&final_product,1,MPI_INT,MPI_PROD,0,MPI_COMM_WORLD);

	
	return final_product;
}

int max(int array[], int n, int size, int rank){

	int max = INT_MIN, final_max = INT_MIN;
	int * scattered_array;

	scattered_array=(int *)calloc(n/size, sizeof(int));

	MPI_Scatter(array, n/size, MPI_INT, scattered_array, n/size, MPI_INT, 0, MPI_COMM_WORLD);

	for(int i=0;i<n/size;i++)
	{
		if (scattered_array[i] > max) max=scattered_array[i];
	}

	MPI_Reduce(&max,&final_max,1,MPI_INT,MPI_MAX,0,MPI_COMM_WORLD);

	return final_max;
}

int min(int array[], int n, int size, int rank){
	
	int min = INT_MAX, final_min = INT_MAX;
	int * scattered_array;

	scattered_array=(int *)calloc(n/size, sizeof(int));

	MPI_Scatter(array, n/size, MPI_INT, scattered_array, n/size, MPI_INT, 0, MPI_COMM_WORLD);

	for(int i=0;i<n/size;i++)
	{
		if (scattered_array[i] < min) min=scattered_array[i];
	}

	MPI_Reduce(&min,&final_min,1,MPI_INT,MPI_MIN,0,MPI_COMM_WORLD);

	return final_min;

}