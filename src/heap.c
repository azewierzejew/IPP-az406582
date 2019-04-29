#include "heap.h"
#include "vector.h"

#include <stdbool.h>


// Deklaracje struktur.

/** Przechowuje kopiec. */
struct HeapStruct {
    /** Komparator definiujący porządek w kopcu. */
    int (*comparator)(void *, void *);

    /** Wektor elementów na kopcu. */
    Vector *elements;
};


// Funkcje pomocnicze.

static void doNothing(__attribute__((unused)) void *arg);


// Implementacja funkcji pomocniczych.

static void doNothing(__attribute__((unused)) void *arg) {

}


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

bool isEmptyHeap(Heap *heap) {
    if (heap == NULL) {
        return true;
    }
    return isEmptyVector(heap->elements);
}

bool addToHeap(Heap *heap, void *value) {
    if (heap == NULL || value == NULL) {
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


    size_t minimumNr = 0;

    for (size_t i = 1; i < elementCount; i++) {
        if (heap->comparator(elements[i], elements[minimumNr]) < 0) {
            minimumNr = i;
        }
    }

    void *minimum = elements[minimumNr];

    // Zamiana miejsc bo popFromVector przegląda od końca.
    elements[minimumNr] = elements[elementCount - 1];
    elements[elementCount - 1] = NULL;
    popFromVector(heap->elements, NULL, doNothing);

    return minimum;
}