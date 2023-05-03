#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

#define N 128
#define base 0

int main (int argc, char *argv[]) {
    
    MPI_Init( &argc, &argv );
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    char * buffer;
    long file_size;
    long i, j,*freq;

    if (rank==0){
        FILE *pFile;
        char * filename;
        size_t result;

        if (argc != 2) {
            printf ("Usage : %s <file_name>\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
        filename = argv[1];
        pFile = fopen ( filename , "rb" );
        if (pFile==NULL) {
            printf ("File error\n"); 
            MPI_Abort(MPI_COMM_WORLD, 2);
            return 2;
        }

        

        // obtain file size:
        fseek (pFile , 0 , SEEK_END);
        file_size = ftell (pFile);
        rewind (pFile);
        printf("file size is %ld\n", file_size);
        
        // allocate memory to contain the file:
        buffer = (char*) malloc (sizeof(char)*file_size);

        // copy the file into the buffer:
        result = fread (buffer,1,file_size,pFile);

        MPI_Bcast(&file_size, 1, MPI_LONG, 0, MPI_COMM_WORLD);
        
        //scatter the array
        MPI_Scatter(buffer, file_size/size, MPI_CHAR, buffer, file_size/size, MPI_CHAR, 0, MPI_COMM_WORLD);

        //close file
        fclose (pFile);

    }
    else{

        MPI_Bcast(&file_size, 1, MPI_LONG, 0, MPI_COMM_WORLD);
        buffer = (char*) malloc (sizeof(char)*file_size/size);

        MPI_Scatter(buffer, file_size/size, MPI_CHAR, buffer, file_size/size, MPI_CHAR, 0, MPI_COMM_WORLD);
        
    }


    long *freq_local;
    freq_local = (long*) malloc (N*sizeof(long));
    memset(freq_local, 0, N*sizeof(long));

    //count the frequency
	for (i=0; i<file_size; i++){
		freq_local[buffer[i] - base]++;
	}
    
    if (rank==0){
        freq = (long*) malloc (N*sizeof(long));
        memset(freq, 0, N*sizeof(long));
    }
    //reduce the frequency array
    MPI_Reduce(freq_local, freq, N, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    //print the frequency
    if (rank==0){
    	for (j=0; j<N; j++){
    		printf("%ld = %ld\n", j+base, freq[j]);
        }
    }

    MPI_Finalize();
    return 0;
}