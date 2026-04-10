#pragma once
#include <stdint.h>
#include <stdbool.h>

#define FILTER_QUEUE_CAPACITY 16

typedef struct FilterQueueF
{
    float data[FILTER_QUEUE_CAPACITY];
    int32_t front;
    int32_t rear;
} FilterQueueF;

#define FilterQueue_Initialize(queue) \
    do                                \
    {                                 \
        queue.front = -1;             \
        queue.rear = -1;              \
    } while (0)

bool FilterQueueF_IsFull(const FilterQueueF *queue);

bool FilterQueueF_IsEmpty(const FilterQueueF *queue);

int32_t FilterQueueF_Count(const FilterQueueF *queue);

// returns the median
float FilterQueueF_Enqueue(FilterQueueF *queue, float data);

float FilterQueueF_Dequeue(FilterQueueF *queue);
