#include <iostream>
#include "mpi.h"
#include <fstream>
#include <vector>
#include <string>

void Count_sort(std::vector<int> &a, int start, int end) {
    int i, j, count;
    std::vector<int> temp(a.size());
    for (i = start; i <= end; i++) {
        count = 0;
        for (j = 0; j < a.size(); j++)
            if (a[j] < a[i])
                count++;
            else if (a[j] == a[i] && j < i)
                count++;
        temp[count] = a[i];
    }
    for (int i = 0; i<temp.size(); i++)
        a[i] = temp[i];
}


void masterInit(std::vector<int> &a,int argc, char *argv[]){
    //if -b flag is passed then use random numbers in range 0-1000 for benchmarking
    long n;
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
    if (rank == 0){
        masterInit(a,argc, argv);
    }
    printArray(a);

    Count_sort(a, 1, 4);

    printArray(a);

    

    //save results to file
    if (rank == 0){
        masterSave(a);
    }
    

    MPI_Finalize();
    return 0;
}
