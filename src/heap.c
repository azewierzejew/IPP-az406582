#include "heap.h"
#include "vector.h"

#include <stdbool.h>


// Deklaracje struktur.


struct HeapStruct {
    int (*comparator)(void *, void *);

    Vector *elements;
};

// Funkcje pomocnicze.




// Implementacja funkcji pomocniczych.




// Funkcje z interfejsu.

Heap *initHeap(int comparator(void *, void *)) {
    Heap *heap = malloc(sizeof(Heap));
    if (heap == NULL) {
        return NULL;
    }

    heap->elements = initVector();
    if (heap->elements == NULL) {
        free(heap);
        return NULL;
    }

    heap->comparator = comparator;
    return heap;
}

void deleteHeap(Heap *heap, void valueDestructor(void *)) {
    if (heap == NULL) {
        return;
    }

    deleteVector(heap->elements, valueDestructor);
    free(heap);
}

bool isEmpty(Heap *heap) {
    if (heap == NULL) {
        return true;
    }
    return isEmptyVector(heap->elements);
}

bool addToHeap(Heap *heap, void *value) {
    if (heap == NULL) {
        return false;
    }
    return pushToVector(heap->elements, value);
}

void *getMinimumFromHeap(Heap *heap) {
    if (heap == NULL) {
        return NULL;
    }

    size_t elementCount = sizeOfVector(heap->elements);
    void **elements = storageBlockOfVector(heap->elements);

    if (elementCount == 0) {
        return NULL;
    }

    void *minimum = elements[0];

    for (size_t i = 1; i < elementCount; i++) {
        if (heap->comparator(elements[i], minimum) < 0) {
            minimum = elements[i];
        }
    }

    return minimum;
}



