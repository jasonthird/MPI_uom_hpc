#include <iostream>
#include <fstream>
#include <vector>
#include "mpi.h"
#include <memory>
#include <algorithm>

#define N 128
#define base 0

int main (int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::vector<char> buffer;
    std::size_t file_size;
    std::vector<long> freq_local(N);

    if (rank == 0) {
        std::string filename;
        if (argc != 2) {
            std::cerr << "Usage : " << argv[0] << " <file_name>\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
        filename = argv[1];
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            std::cerr << "File error\n";
            MPI_Abort(MPI_COMM_WORLD, 2);
            return 2;
        }
        // obtain file size:
        file.seekg(0, std::ios::end);
        file_size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::cout << "file size is " << file_size << std::endl;

        // allocate memory to contain the file:
        std::vector<char> bufferFile(file_size);
        // copy the file into the buffer:
        file.read(bufferFile.data(), file_size);

        MPI_Bcast(&file_size, 1, MPI_LONG, 0, MPI_COMM_WORLD);
        buffer.resize(file_size / size);
        // scatter the array
        MPI_Scatter(bufferFile.data(), file_size / size, MPI_CHAR, buffer.data(), file_size / size, MPI_CHAR, 0,
                    MPI_COMM_WORLD);
    } else {
        MPI_Bcast(&file_size, 1, MPI_LONG, 0, MPI_COMM_WORLD);
        buffer.resize(file_size / size);
        MPI_Scatter(nullptr, file_size / size, MPI_CHAR, buffer.data(), file_size / size, MPI_CHAR, 0, MPI_COMM_WORLD);
    }

    // count the frequency
    //cast the frequency local to an array
    long *freq_local_data = freq_local.data();
    #pragma omp parallel for reduction(+:freq_local_data[:N]) firstprivate(buffer)
    for (int i=0; i<buffer.size(); i++){
        freq_local_data[buffer[i] - base]++;
    }

    std::unique_ptr<long[]> freq;
    if (rank == 0) {
        freq = std::make_unique<long[]>(N);
    }
    // reduce the frequency array
    MPI_Reduce(freq_local.data(), freq.get(), N, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    // print the frequency
    if (rank == 0) {
        for (std::size_t j = 0; j < N; ++j) {
            std::cout << j + base << " = " << freq[j] << std::endl;
        }
    }

    MPI_Finalize();
    return 0;
}
