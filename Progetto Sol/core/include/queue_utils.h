#ifndef QUEUE_H
#define QUEUE_H

typedef struct Queue {
    void * value;
    struct Queue *next;
} Queue;

void create_queue(Queue **queue, long (*compare) (void *, void *));
int queue_sorted_insert(Queue **queue, void *data);
int queue_push(Queue **queue, void *value);
void *queue_pop(Queue **queue);
size_t len_queue(Queue *queue);
void free_queue(Queue **queue);


#endif