#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int main( int argc, char *argv[] )
{
    int rank, size, tmp;
    MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    
    if (argc != 2) 
    {
	printf ("Usage : %s <number>\n", argv[0]);
	return 1;
    }
    tmp = strtol(argv[1], NULL, 10);
          
    printf( "Hello world %d from process %d of %d\n", tmp, rank, size );
    
    MPI_Finalize(); 
    return 0; 
}