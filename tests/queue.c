#include "dataFilter.h"
#include <stdio.h>

void printQueue(const FilterQueueF *queue)
{
    printf("queue (s: %d) (f: %d) (r: %d) : [", FilterQueueF_Count(queue), queue->front, queue->rear);

    for (int i = queue->front; i != queue->rear; i = (i + 1) % FILTER_QUEUE_CAPACITY)
    {
        printf("%f, ", queue->data[i]);
    }

    printf("%f]\n", queue->data[queue->rear]);
}

int main(int argc, char **argv)
{
    FilterQueueF queue = {0};
    FilterQueue_Initialize(queue);

    printQueue(&queue);
    FilterQueueF_Enqueue(&queue, 1.0f);
    printQueue(&queue);
    FilterQueueF_Enqueue(&queue, 2.0f);
    printQueue(&queue);
    FilterQueueF_Enqueue(&queue, 3.0f);
    printQueue(&queue);
    FilterQueueF_Dequeue(&queue);
    printQueue(&queue);
    FilterQueueF_Enqueue(&queue, 4.0f);
    printQueue(&queue);
    FilterQueueF_Enqueue(&queue, 5.0f);
    printQueue(&queue);
    FilterQueueF_Dequeue(&queue);
    printQueue(&queue);
    FilterQueueF_Dequeue(&queue);
    printQueue(&queue);
    FilterQueueF_Dequeue(&queue);
    printQueue(&queue);
}