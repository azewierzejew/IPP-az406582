#ifndef DROGI_HEAP_H
#define DROGI_HEAP_H

#include <stdbool.h>

struct HeapStruct;

typedef struct HeapStruct Heap;

Heap *initHeap(int comparator(void *, void *));

void deleteHeap(Heap *heap, void valueDestructor(void *));

bool isEmpty(Heap *heap);

bool addToHeap(Heap *heap, void *value);

void *getMinimumFromHeap(Heap *heap);

#endif //DROGI_HEAP_H
