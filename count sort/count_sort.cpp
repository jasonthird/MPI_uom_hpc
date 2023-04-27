//when using openmp use mpirun or mpiexec --bind-to none  
#include <iostream>
#include "mpi.h"
#include <fstream>
#include <vector>
#include <string>

std::vector<std::pair<int,int>> Count_sort(std::vector<int> &a, int start, int end) {
    int i, j, count;
    std::vector<std::pair<int,int>> NewIndex;
    NewIndex.reserve(end - start + 1);
    #pragma omp parallel for private(i, j, count) shared(a, NewIndex) firstprivate(start, end)
    for (i = start; i <= end; i++) {
        count = 0;
        for (j = 0; j < a.size(); j++)
            if (a[j] < a[i])
                count++;
            else if (a[j] == a[i] && j < i)
                count++;
        NewIndex.emplace_back(std::make_pair(count, a[i]));
    }
    return NewIndex;
}


void masterInit(std::vector<int> &a,int argc, char *argv[]){
    int n = 0;
    //if -b flag is passed then use random numbers in range 0-1000 for benchmarking
    if (argc == 2){
        std::cout << "please specify the size of the array to sort" << std::endl;
    } else if (argc ==3) {
        srand (time(NULL));
        switch(argv[1][0]){
            case '-':

                switch(argv[1][1]){
                    case 'b':
                        n = std::stol(argv[2]);
                        a.resize(n);
                        for (int i = 0; i < n; i++){
                            a[i] = (rand() % (1000 + 1));
                        }
                    break;
                    default:
                        std::cout << "invalid flag" << std::endl;
                    break;
                }
                break;
            default:
                std::cout << "invalid flag" << std::endl;
                break;
        }
    }
    else{
        //read from file and store in a vector
        std::ifstream input("input.txt");
        
        std::string line;
        
        if (input.is_open()){
            while (getline(input, line)){
                a.emplace_back(std::stoi(line));
            }
            input.close();
        }
        else std::cout << "Unable to open file";
    }
}

void masterSave(std::vector<int> &a){
    std::ofstream output("output.txt");
    std::cout<< "saving results to file" << std::endl;
    if (output.is_open()){
        for (int i = 0; i < a.size(); i++){
            output << a[i] << std::endl;
        }
        output.close();
    }
    else {
        std::cout << "Unable to open file";
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
}

void printArray(std::vector<int> &a){
    for (int i = 0; i < a.size(); i++){
        std::cout << a[i] << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char *argv[]){

    MPI_Init( &argc, &argv );
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    //initialize the array and read from file or generate random numbers
    std::vector<int> a;
    long n = 0;
    if (rank == 0){
        masterInit(a, argc, argv);
        // broadcast the size of the array to slaves
        n = a.size();
        MPI_Bcast(&n, 1, MPI_LONG, 0, MPI_COMM_WORLD);
        // broadcast the array to slaves
        MPI_Bcast(a.data(), a.size(), MPI_INT, 0, MPI_COMM_WORLD);
    } else {
        MPI_Bcast(&n, 1, MPI_LONG, 0, MPI_COMM_WORLD);
        a.resize(n);
        MPI_Bcast(a.data(), a.size(), MPI_INT, 0, MPI_COMM_WORLD);
    }

    //split the array into chunks
    int chunk_size = a.size() / size;
    int start = rank * chunk_size;
    int end = start + chunk_size - 1;

    //sort the chunks
    std::vector<std::pair<int,int>> indexes = Count_sort(a, start, end);

    //send the chunks to the master
    if (rank != 0){
        MPI_Send(&indexes[0], indexes.size() * 2, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    //receive the chunks from the slaves and merge them
    if (rank == 0){
        for (int i = 1; i < size; i++){
            std::vector<std::pair<int,int>> temp(chunk_size);
            MPI_Recv(&temp[0], chunk_size * 2, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            indexes.insert(indexes.end(), temp.begin(), temp.end());
        }
    }

    //check if there are any elements left and sort them if there are
    if (rank == 0){
        if (a.size() % size != 0){
            int start = a.size() - (a.size() % size);
            int end = a.size() - 1;
            std::vector<std::pair<int,int>> temp = Count_sort(a, start, end);
            indexes.insert(indexes.end(), temp.begin(), temp.end());
        }
    }

    //merge the results
    if(rank == 0){
        #pragma omp parallel for shared(a) firstprivate(indexes)
        for (int i = 0; i < indexes.size(); i++){
            a[indexes[i].first] = indexes[i].second;
        }
    }

    //save results to file
    if (rank == 0){
        masterSave(a);
    }

    MPI_Finalize();
    return 0;
}
