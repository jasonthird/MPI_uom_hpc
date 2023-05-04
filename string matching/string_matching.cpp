#include <iostream>
#include <fstream>
#include <vector>
#include "mpi.h"
#include <memory>

#define N 128
#define base 0

int main (int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::vector<char> buffer;
    std::size_t file_size;

    if (rank == 0) {
        std::string filename;
        if (argc != 3) {
            std::cerr << "Usage : " << argv[0] << " <file_name> <string>\n";
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

        int pattern_size = std::string(argv[2]).size();
    
        // scatter the array using MPI_Scatterv
        // send the chunks plus the pattern size to each process
        std::vector<int> sendcounts(size);
        std::vector<int> displs(size);
        for (int i = 0; i < size; ++i) {
            sendcounts[i] = file_size / size + pattern_size;
            displs[i] = i * file_size / size;
        }
        if (file_size % size != 0) {
            sendcounts[size - 1] += file_size % size + pattern_size;
        }
        //sent the sendcounts to all the processes
        for (int i = 1; i < size; ++i){
            int temp = sendcounts[i];
            MPI_Send(&temp, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
        
        buffer.resize(sendcounts[rank]);
        MPI_Scatterv(bufferFile.data(), sendcounts.data(), displs.data(), MPI_CHAR, buffer.data(), sendcounts[rank], MPI_CHAR, 0, MPI_COMM_WORLD);

    } else {
        //receive the sendcounts from the root
        int sendcounts;
        MPI_Recv(&sendcounts, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        buffer.resize(sendcounts);
        MPI_Scatterv(nullptr, nullptr, nullptr, MPI_CHAR, buffer.data(), sendcounts, MPI_CHAR, 0, MPI_COMM_WORLD);
    }

    //get pattern to search
    std::string pattern;
    pattern = argv[2];
    // std::cout << "pattern is " << pattern << std::endl;
    // std::cout << "rank " << rank << " has " << buffer.size() << " characters\n";

    //search for the pattern
    std::vector<int> match(buffer.size()-pattern.size()+1, 0);
    int total_matches = 0;
    int i,j;
    for (j=0; j < buffer.size(); ++j){
        for (i = 0; i < pattern.size() && pattern[i] == buffer[i + j]; ++i);
        if (i >= pattern.size()) {
        		match[j]++;
         		total_matches++;
        }	
    }

    //reduce total_matches
    int total_matches_all;
    MPI_Reduce(&total_matches, &total_matches_all, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    if (rank == 0){
        std::cout << "Total matches: " << total_matches_all << std::endl;
    }

    //construct the match vector
    std::vector<int> match_all;
    if (rank == 0){
        match_all.resize(file_size);
    }
    
    //construct the sendcounts from each nice slave and send to master
    std::vector<int> sendcounts(size);
    std::vector<int> displs(size);
    if (rank==0){
        sendcounts[0] = match.size();
        displs[0] = 0;
        for (int i = 1; i < size; ++i){
            MPI_Recv(&sendcounts[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            displs[i] = displs[i-1] + sendcounts[i-1];
        }
    }
    else{
        int size_match = match.size();
        MPI_Send(&size_match, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    //gather the match vector from each slave
    MPI_Gatherv(match.data(), match.size(), MPI_INT, match_all.data(), sendcounts.data(), displs.data(), MPI_INT, 0, MPI_COMM_WORLD);
    

    //print the matches
    if (rank == 0){
        for (int i = 0; i < match_all.size(); ++i){
            if (match_all[i] > 0){
                std::cout << "Match at position " << i << std::endl;
            }
        }
    }

    MPI_Finalize();
    return 0;
}