#include <fstream>
#include "mpi.h"

int main(int argc, char *argv[]){
    
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);

    if (argc != 3) {
        if (rank == 0) {
            std::cerr << "Usage : " << argv[0] << " <file_name> <string>" << std::endl;
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
        return 1;
    }

}