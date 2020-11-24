#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h>


int  MPI_Merge_sort(int n, double * array,  int root, MPI_Comm comm);

int is_sorted(double *a, int n);

int isActive(int rank, int p, int l);
int isReciever(int rank, int p, int l);
int isSender(int rank, int p, int l);

double* merge_array(int n, double* a, int m, double* b);
void merge_sort(int n, double* a);
void swap(double* a, double* b);

int main(int argc, char* argv[])
{

	int rank, size;

	int n = 16, q, l, i, j, k, x, * nr;
	double m = 10.0;
	double* a, * b;

	a = (double *) calloc(n,sizeof(double));
	b = (double *) calloc(n,sizeof(double));

	MPI_Status status;

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	
	
	if (rank == 0)
	{
		//initialise the array with random values, then scatter to all processors
		srand(((unsigned)time(NULL) + rank));

		for (x = 0; x < n; x++)
		{
			a[x] = ((double)rand() / RAND_MAX) * m;
			printf("Initial: %f\n", a[x]);
		}

	}


	/*
	//find what power of 2, the number of processors is..
	//may cause errors if no of processors is not a exact power of 2

	q = 0;
	while (size > (pow(2, q)))
	{
		q++;
	}

	// --------------- Stage 1: top-down -----------------------------

	for (l = 0; l <= q; l++)
	{
		//if is Reciever
		if (isReciever(rank, size, l) && l > 0)
		{
			// receive the elements from the parent node
			MPI_Recv(a, n / (int)pow(2, l), MPI_DOUBLE, rank - size / (int)pow(2, l),
				0, MPI_COMM_WORLD, &status);


		}
		//if is Active  ,
		if (isActive(rank, size, l) && l < q)
		{
			// send half of the elements to the right child node
			MPI_Send(&a[n / (int)pow(2, l+1)], n / (int)pow(2, l + 1),
				MPI_DOUBLE, rank + size / (int)pow(2, l + 1), 0, MPI_COMM_WORLD);
			
			

		}
	}

	// --------------- Stage 2: sort the arrays -----------------------------

	//sequential sort the scattered array



	merge_sort(n / (int)pow(2, q), a);

	// --------------- Stage 3: bottom-up -----------------------------
	for (l = q; l >= 0; l--)
	{
		if (isActive(rank, size, l) && l < q)
		{
			// receive the elements from the right child in the array b and then merge a with 
			MPI_Recv(b, n / (int)pow(2, l), MPI_DOUBLE,
				rank + size / (int)pow(2, l + 1), 0, MPI_COMM_WORLD, &status);
	
			a = merge_array(n / (int)pow(2, l+1), a, n / (int)pow(2, l+1), b);

		}
		if (isSender(rank, size, l) && l > 0)
		{
			// send the elements to the parent node.
			MPI_Send(a, n / (int)pow(2, l -1 ), MPI_DOUBLE,
				rank - size / (int)pow(2, l), 0, MPI_COMM_WORLD);
		}
	}
	*/

	MPI_Merge_sort(n, a,  0, MPI_COMM_WORLD);


	if (rank == 0)
	{
		printf( is_sorted(a,n)? "sorted\n": "Not Sorted\n"   );
		for (x = 0; x < n; x++)
		{
			printf("Output : %f\n", a[x]);
		}
	}

	
	MPI_Finalize();

}

int  MPI_Merge_sort(int n, double * array,  int root, MPI_Comm comm){

  	int level, q;

  	int rank, size;
  	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm, &size);

  	double * b;
	b = (double *) calloc(n,sizeof(double));
  	

  	MPI_Status status;


	if (rank == 0)
	{
		//initialise the array with random values, then scatter to all processors

		for (int x = 0; x < n; x++)
		{
			printf("Passed: %f\n", array[x]);
		}

	}


	//find what power of 2, the number of processors is..
	//may cause errors if no of processors is not array exact power of 2

	q = 0;
	while (size > (pow(2, q)))
	{
		q++;
	}

	// --------------- Stage 1: top-down -----------------------------

	for (level = 0; level <= q; level++)
	{
		//if is Reciever
		if (isReciever(rank, size, level) && level > 0)
		{
			// receive the elements from the parent node
			MPI_Recv(array, n / (int)pow(2, level), MPI_DOUBLE, rank - size / (int)pow(2, level),
				0, comm, &status);


		}
		//if is Active  ,
		if (isActive(rank, size, level) && level < q)
		{
			// send half of the elements to the right child node
			MPI_Send(&array[n / (int)pow(2, level+1)], n / (int)pow(2, level + 1),
				MPI_DOUBLE, rank + size / (int)pow(2, level + 1), 0, comm);
		}
	}

	// --------------- Stage 2: sort the arrays -----------------------------

	//sequential sort the scattered array
	merge_sort(n / (int)pow(2, q), array);

	// --------------- Stage 3: bottom-up -----------------------------
	for (level = q; level >= 0; level--)
	{
		if (isActive(rank, size, level) && level < q)
		{
			// receive the elements from the right child in the array b and then merge array with
			MPI_Recv(b, n / (int)pow(2, level), MPI_DOUBLE,
				rank + size / (int)pow(2, level + 1), 0, comm, &status);

			array = merge_array(n / (int)pow(2, level+1), array, n / (int)pow(2, level+1), b);

		}
		if (isSender(rank, size, level) && level > 0)
		{
			// send the elements to the parent node.
			MPI_Send(array, n / (int)pow(2, level -1 ), MPI_DOUBLE,
				rank - size / (int)pow(2, level), 0, comm);
		}
	}

return MPI_SUCCESS;

}





// ------------------------------------------------------------
//
// these functions test if a node is active, sender of receiver
//


int isActive(int rank, int p, int l)
{
	if ((rank % ((int)(p / pow(2, l)))) == 0) return 1;
	return 0;
}

int isReciever(int rank, int p, int l)
{
	if (!isActive(rank, p, l)) return 0;
	int result = rank / ((int)(p / pow(2, l)));
	if (result % 2 == 1) return 1;
	return 0;
}

int isSender(int rank, int p, int l)
{
	return isReciever(rank, p, l);
}



// ------------------------------------------------------------
//
// these functions deal with sorting and merging
//

double* merge_array(int n, double* a, int m, double* b) {

	int i, j, k;
	double* c = (double*)calloc(n + m, sizeof(double));

	for (i = j = k = 0; (i < n) && (j < m);)
		if (a[i] <= b[j])c[k++] = a[i++];
		else c[k++] = b[j++];
	if (i == n)for (; j < m;)c[k++] = b[j++];
	else for (; i < n;)c[k++] = a[i++];
	return c;
}

void merge_sort(int n, double* a) {

	double* c;
	int i;

	if (n <= 1) return;

	if (n == 2) {

		if (a[0] > a[1])swap(&a[0], &a[1]);
		return;
	}



	merge_sort(n / 2, a); merge_sort(n - n / 2, a + n / 2);

	c = merge_array(n / 2, a, n - n / 2, a + n / 2);

	for (i = 0; i < n; i++)a[i] = c[i];

}

void swap(double* a, double* b) {

	double temp;

	temp = *a; *a = *b; *b = temp;

}

int is_sorted(double *a, int n){

	for(int i =0; i<n-1; i++){
		if (a[i] > a[i+1]) return 0;

	}


	return 1;
}