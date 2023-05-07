import random, time
from proj.tasks import MergeSort

def checkIfSorted(array):
    for i in range(len(array) - 1):
        if array[i] > array[i + 1]:
            return False
    return True

if __name__ == '__main__':
    #fill an array with random numbers
    array = [random.randint(0, 1000) for i in range(2000000)]

    #get time
    startTime = time.time()
    sortedArray = MergeSort.delay(array).get()
    finishTime = time.time()
    print("Time taken: " + str(finishTime - startTime))
    print("Sorted: " + str(checkIfSorted(sortedArray)))
