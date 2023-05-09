#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mpi.h"
#include <vector>
#include <algorithm>
#include <random>

void generate_list(int * x, int n) {
    int i;
    srand (time (NULL));
    for (i = 0; i < n; i++){
        x[i] = rand() % n; 
    }
        
}


void MergeSort(std::vector<int> &vector) {
    if (vector.size() < 65536) {
        std::sort(vector.begin(), vector.end());
        return;
    }
    int middle = vector.size() / 2;
    std::vector<int> leftVector(middle);
    std::vector<int> rightVector(vector.size() - middle);
    for (int i = 0; i < middle; i++) {
        leftVector[i] = vector[i];
    }
    for (int i = middle; i < vector.size(); i++) {
        rightVector[i - middle] = vector[i];
    }
    MergeSort(leftVector);
    MergeSort(rightVector);
    std::merge(leftVector.begin(), leftVector.end(), rightVector.begin(), rightVector.end(), vector.begin());

    //free up memory
    leftVector.clear();
    rightVector.clear();
}


int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);

    int n;
    std::vector<int> data;
    
    if (argc != 2) {
        if(rank==0){
            printf ("Usage : %s <list size>\n", argv[0]);
        }
        MPI_Finalize();
        return 0;
    }
    if (rank == 0){

        n = strtol(argv[1], NULL, 10);
        data.resize(n);
        generate_list(data.data(), n);
    }

    //scattering the data using MPI_Scatterv
    std::vector<int> sendcounts(size);
    std::vector<int> displs(size);
    if(rank==0){
        displs[0] = 0;
        sendcounts[0] = data.size() / size;
        for (int i = 1; i < size; ++i) {
            sendcounts[i] = data.size() / size;
            displs[i] = displs[i-1] + sendcounts[i-1];
        }

        if (data.size() % size != 0) {
            sendcounts[size - 1] += data.size() % size;
        }
        MPI_Bcast(sendcounts.data(), size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(displs.data(), size, MPI_INT, 0, MPI_COMM_WORLD);
    }else{
        MPI_Bcast(sendcounts.data(), size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(displs.data(), size, MPI_INT, 0, MPI_COMM_WORLD);
    }


    std::vector<int> local_data(sendcounts[rank]);

    MPI_Scatterv(data.data(), sendcounts.data(), displs.data(), MPI_INT, local_data.data(), sendcounts[rank], MPI_INT, 0, MPI_COMM_WORLD);
    MergeSort(local_data);

    //merging sublists
    std::vector<int> sorted_data;
    if(rank==0){
        sorted_data.resize(data.size());
    }

    //recive data
    MPI_Gatherv(local_data.data(),sendcounts[rank],MPI_INT,sorted_data.data(),sendcounts.data(),displs.data(),MPI_INT,0,MPI_COMM_WORLD);
    
    //print receved data
    // if(rank==0){
    //     for (int i = 0; i < sorted_data.size(); i++) {
    //             printf("%d ", sorted_data[i]);
    //     }
    //         std::cout<<std::endl;
    // }

    //merge sorted sublists
    if(rank==0){
        for(int i=0;i<size;i++){
            std::inplace_merge(sorted_data.begin(), sorted_data.begin() + displs[i], sorted_data.begin() + displs[i] + sendcounts[i]);
        }
    }
    
    // //print sorted list
    // for (int i = 0; i < sorted_data.size(); i++) {
    //     printf("%d ", sorted_data[i]);
    // }

    if(rank==0){
        if (std::is_sorted(sorted_data.begin(), sorted_data.end())) {
            printf("List is sorted!\n");
        } else {
            printf("List is not sorted!\n");
        }
    }

    // printf("\nList After Sorting...\n");
    // print_list(data, n);
    // printf("\n");
    MPI_Finalize();
    return 0;
}