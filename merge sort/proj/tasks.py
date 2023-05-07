from .celery import app
from celery.result import allow_join_result
from celery import group , chord

@app.task()
def merge(refs):
    leftArray = refs[0]
    rightArray = refs[1]
    merged = []
    while len(leftArray) and len(rightArray):
        if leftArray[-1] > rightArray[-1]:
            merged.append(leftArray.pop())
        else:
            merged.append(rightArray.pop())

    merged.extend(leftArray)
    merged.extend(rightArray)
    return merged[::-1]

@app.task()
def MergeSort(array):
    
    #works but too slow
    # if len(array) < 2:
    #     return array
    
    # ill just use this to speed things up
    if len(array) < 100000:
        return sorted(array)

    middle = len(array) // 2

    # #with chords, needs redis to work

    # c = chord([MergeSort.s(array[:middle]), MergeSort.s(array[middle:])])(merge.s())
    # with allow_join_result():
    #     return c.get()
    

    #with groups, works a bit faster than chords since I can call local merge function
    #works with rabbitmq and redis

    tasks = group([MergeSort.s(array[:middle]), MergeSort.s(array[middle:])])
    tasks = tasks.apply_async()
    with allow_join_result():
        result = merge(tasks.get())
    return result
    
    