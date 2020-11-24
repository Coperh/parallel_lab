#include "mpi.h"
#include <stdio.h>
#include <math.h>

int isprime(long n);

int main(int argc, char ** argv){

    int n=10000000,i,size,rank, count=0;

    double time;	

    MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );

    time=MPI_Wtime();



	for(i = 3 + (rank *2); i<n; i=i+size*2){
        if (isprime(i)) count++;
    }
        

    time=MPI_Wtime()-time;
	
	
	
    printf("Processor %d finds %d primes in %lf\n",rank,count,time);    
    
   

    MPI_Finalize();
}


int isprime(long n){
    int d, ans=1;
    if(n==1)return 0;
    if(n==2||n==3)return 1;
    if(n%2==0)return 0;
    for(d=3;d<=sqrt(n);d+=2)
        if(n%d==0){ans=0;break;}
    return ans;    
}