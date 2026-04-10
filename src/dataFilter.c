#include "dataFilter.h"
#include <string.h>

bool FilterQueueF_IsFull(const FilterQueueF *queue)
{
    return ((queue->rear + 1) % FILTER_QUEUE_CAPACITY) == queue->front;
}

bool FilterQueueF_IsEmpty(const FilterQueueF *queue)
{
    return queue->front == -1;
}

int32_t FilterQueueF_Count(const FilterQueueF *queue)
{
    if (FilterQueueF_IsEmpty(queue))
    {
        return 0;
    }

    return (FILTER_QUEUE_CAPACITY - queue->front + queue->rear) % FILTER_QUEUE_CAPACITY + 1;
}

float FilterQueueF_Enqueue(FilterQueueF *queue, float data)
{
    if (FilterQueueF_IsEmpty(queue))
    {
        queue->front = 0; // first element

        queue->rear = (queue->rear + 1) % FILTER_QUEUE_CAPACITY;
        queue->data[queue->rear] = data;

        goto calculateMedian;
    }

    queue->rear = (queue->rear + 1) % FILTER_QUEUE_CAPACITY;
    queue->data[queue->rear] = data;

    if (queue->front == queue->rear) // override if full
    {
        queue->front = (queue->rear + 1) % FILTER_QUEUE_CAPACITY;
    }

calculateMedian:
    int32_t arrayCount = FilterQueueF_Count(queue);

    float tempArray[FILTER_QUEUE_CAPACITY] = {0};
    int32_t currentIdx = queue->front;
    for (int32_t i = 0; i < arrayCount; i++)
    {
        tempArray[i] = queue->data[currentIdx];
        currentIdx = (currentIdx + 1) % FILTER_QUEUE_CAPACITY;
    }

    for (int i = 0; i < arrayCount - 1; i++)
    {
        for (int j = i + 1; j < arrayCount; j++)
        {
            if (tempArray[j] < tempArray[i])
            {
                float temp = tempArray[i];
                tempArray[i] = tempArray[j];
                tempArray[j] = temp;
            }
        }
    }

    return tempArray[arrayCount / 2];
}

float FilterQueueF_Dequeue(FilterQueueF *queue)
{
    float data = queue->data[queue->front];

    if (queue->front == queue->rear) // queue is now empty
    {
        queue->front = -1;
        queue->rear = -1;
    }
    else
    {
        queue->front = (queue->front + 1) % FILTER_QUEUE_CAPACITY;
    }

    return data;
}
