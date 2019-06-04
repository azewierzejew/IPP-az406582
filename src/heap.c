#include "heap.h"
#include "vector.h"
#include "map_basics.h"

#include <stdbool.h>


/* Deklaracje struktur. */

/** Przechowuje kopiec. */
struct HeapStruct {
    /** Komparator definiujący porządek w kopcu. */
    int (*comparator)(void *, void *);

    /** Wektor elementów na kopcu. */
    Vector *elements;
};


/* Funkcje pomocnicze. */

static void swap(void **array, size_t index1, size_t index2) {
    void *tmp = array[index1];
    array[index1] = array[index2];
    array[index2] = tmp;
}

static int heapCompare(Heap *heap, void *element1, void *element2) {
    if (heap == NULL || (element1 == NULL && element2 == NULL)) {
        return 0;
    }
    /* Jeśli jeden to NULL to uznajemy go za większego, czyli nie będzie wypychany na górę. */
    if (element1 == NULL) {
        return 1;
    }
    if (element2 == NULL) {
        return -1;
    }

    return heap->comparator(element1, element2);
}


/* Funkcje z interfejsu. */

Heap *initHeap(int comparator(void *, void *)) {
    if (comparator == NULL) {
        return false;
    }

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

bool isEmptyHeap(const Heap *heap) {
    if (heap == NULL) {
        return true;
    }
    return isEmptyVector(heap->elements);
}

bool addToHeap(Heap *heap, void **valuePtr) {
    if (heap == NULL || valuePtr == NULL || *valuePtr == NULL) {
        return false;
    }

    void *value = *valuePtr;
    if (!pushToVector(heap->elements, value)) {
        return false;
    }

    size_t position = sizeOfVector(heap->elements) - 1;
    void **elements = storageBlockOfVector(heap->elements);
    while (position > 0) {
        size_t upPosition = (position - 1) / 2;
        if (heapCompare(heap, elements[position], elements[upPosition]) < 0) {
            swap(elements, position, upPosition);
            position = upPosition;
        } else {
            break;
        }
    }
    *valuePtr = NULL;
    return true;
}

void *getMinimumFromHeap(Heap *heap) {
    if (heap == NULL) {
        return NULL;
    }
    size_t elementCount = sizeOfVector(heap->elements);
    if (elementCount == 0) {
        return NULL;
    }

    void **elements = storageBlockOfVector(heap->elements);

    void *minimum = elements[0];
    swap(elements, 0, elementCount - 1);
    popFromVector(heap->elements, minimum, NULL);
    elementCount--;

    size_t position = 0;
    while (position < elementCount) {
        size_t son1Position = position * 2 + 1;
        size_t son2Position = position * 2 + 2;

        void *current = elements[position];
        void *son1 = (son1Position < elementCount) ? elements[son1Position] : NULL;
        void *son2 = (son2Position < elementCount) ? elements[son2Position] : NULL;

        bool biggerThanSon1 = heapCompare(heap, current, son1) > 0;
        bool biggerThanSon2 = heapCompare(heap, current, son2) > 0;

        /* Zamiana z pierwszym jeśli nie da się zamienić z drugim albo drugi nie jest lepszy. */
        if (biggerThanSon1 && (!biggerThanSon2 || heapCompare(heap, son1, son2) < 0)) {
            swap(elements, position, son1Position);
            position = son1Position;
        } else if (biggerThanSon2) {
            swap(elements, position, son2Position);
            position = son2Position;
        } else {
            break;
        }
    }
    return minimum;
}