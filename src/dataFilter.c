#include "dataFilter.h"
#include <string.h>
#include <math.h>

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
    int32_t queueCount = FilterQueueF_Count(queue);

    float tempArray[FILTER_QUEUE_CAPACITY] = {0};
    int32_t currentIndex = queue->front;
    for (int32_t i = 0; i < queueCount; i++)
    {
        tempArray[i] = queue->data[currentIndex];
        currentIndex = (currentIndex + 1) % FILTER_QUEUE_CAPACITY;
    }

    for (int i = 0; i < queueCount - 1; i++)
    {
        for (int j = i + 1; j < queueCount; j++)
        {
            if (tempArray[j] < tempArray[i])
            {
                float temp = tempArray[i];
                tempArray[i] = tempArray[j];
                tempArray[j] = temp;
            }
        }
    }

    if (queueCount % 2 == 0)
    {
        return (tempArray[queueCount / 2 - 1] + tempArray[queueCount / 2]) / 2.0f;
    }
    else
    {
        return tempArray[queueCount / 2];
    }
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

QueueStats FilterQueueF_CalculateStats(const FilterQueueF *queue)
{
    QueueStats stats = {0};
    int32_t queueCount = FilterQueueF_Count(queue);

    if (queueCount == 0)
    {
        return stats;
    }

    float sum = 0.0f;
    float minValue = queue->data[queue->front];
    float maxValue = minValue;

    // Min, Max, Avg
    int32_t currentIndex = queue->front;
    for (int i = 0; i < queueCount; i++)
    {
        float currentValue = queue->data[currentIndex];

        if (currentValue < minValue)
        {
            minValue = currentValue;
        }

        if (currentValue > maxValue)
        {
            maxValue = currentValue;
        }

        sum += currentValue;
        currentIndex = (currentIndex + 1) % FILTER_QUEUE_CAPACITY;
    }
    float avg = sum / queueCount;

    stats.min = minValue;
    stats.max = maxValue;
    stats.avg = avg;

    // Standard Deviation
    float varianceSum = 0.0f;
    currentIndex = queue->front;

    for (int i = 0; i < queueCount; i++)
    {
        float currentValue = queue->data[currentIndex];

        varianceSum += (currentValue - avg) * (currentValue - avg);
        currentIndex = (currentIndex + 1) % FILTER_QUEUE_CAPACITY;
    }
    stats.stdDev = sqrtf(varianceSum / queueCount);

    // Median

    float tempArray[FILTER_QUEUE_CAPACITY] = {0};
    currentIndex = queue->front;
    for (int32_t i = 0; i < queueCount; i++)
    {
        tempArray[i] = queue->data[currentIndex];
        currentIndex = (currentIndex + 1) % FILTER_QUEUE_CAPACITY;
    }

    for (int i = 0; i < queueCount - 1; i++)
    {
        for (int j = i + 1; j < queueCount; j++)
        {
            if (tempArray[j] < tempArray[i])
            {
                float temp = tempArray[i];
                tempArray[i] = tempArray[j];
                tempArray[j] = temp;
            }
        }
    }

    if (queueCount % 2 == 0)
    {
        stats.median = (tempArray[queueCount / 2 - 1] + tempArray[queueCount / 2]) / 2.0f;
    }
    else
    {
        stats.median = tempArray[queueCount / 2];
    }

    return stats;
}
