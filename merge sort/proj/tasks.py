from .celery import app
from celery.result import allow_join_result
import numpy as np

@app.task(task_acks_late=True,worker_prefetch_multiplier=1)
def merge(leftArray, rightArray):
    result = []
    i = 0
    j = 0
    while i < len(leftArray) and j < len(rightArray):
        if leftArray[i] < rightArray[j]:
            result.append(leftArray[i])
            i += 1
        else:
            result.append(rightArray[j])
            j += 1
    result += leftArray[i:]
    result += rightArray[j:]
    return result

@app.task(task_acks_late=True,worker_prefetch_multiplier=1)
def MergeSort(array):
    
    #works but too slow
    # if len(array) < 2:
    #     return array
    
    # ill just use this to speed things up
    if len(array) < 100000:
        return sorted(array)

    middle = len(array) // 2
    
    with allow_join_result():
        leftArray = MergeSort.delay(array[:middle]).get()
        rightArray = MergeSort.delay(array[middle:]).get()
        return merge.delay(leftArray, rightArray).get()
    
    
    