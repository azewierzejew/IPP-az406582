/** @file
 * Implementacja klasy przechowującej minimalny kopiec.
 *
 * @author Antoni Żewierżejew <azewierzejew@gmail.com>
 * @date 29.03.2019
 */

#include "heap.h"
#include "vector.h"

#include <stdbool.h>


/* Deklaracje struktur. */

/** Przechowuje kopiec. */
struct HeapStruct {
    /** Komparator definiujący porządek w kopcu. */
    int (*comparator)(void *, void *);

    /**
     * Wektor elementów na kopcu.
     * Elementy są trzymane na tablicy, ale tworzą binarny kopiec.
     * Synami "wierzchołka" w polu @p i są pola `(2 * i + 1)` i `(2 * i + 2)`.
     */
    Vector *elements;
};


/* Funkcje pomocnicze. */

/**
 * @brief Zamienia w tablicy dwie wartości na podanych pozycjach.
 * @param[in,out] array - wskaźnik na tablicę;
 * @param[in] index1    - indeks pierwszego elementu;
 * @param[in] index2    - indeks drugiego elementu.
 */
static void swap(void **array, size_t index1, size_t index2);

/**
 * @brief Porównuje elementy na kopcu.
 * Porównuje elementy na kopcu korzystając z przypisanego mu komparatora.
 * @p NULL jest uznawany za większy od każdego innego elementu.
 * @param[in] heap     - wskaźnik na kopiec;
 * @param[in] element1 - wskaźnik na pierwszy element;
 * @param[in] element2 - wskaźnik na drugi element.
 * @return @p -1, @p 0, @p 1 w zależności czy pierwszy element jest mniejszy, równy, większy.
 */
static int heapCompare(Heap *heap, void *element1, void *element2);


/* Implementacja funkcji pomocniczych. */

static void swap(void **array, size_t index1, size_t index2) {
    if (array == NULL) {
        return;
    }

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

    /* Dodawana wartość jest na końcu wektora, czyli w jakimś liściu. */
    void *value = *valuePtr;
    if (!pushToVector(heap->elements, value)) {
        return false;
    }

    size_t position = sizeOfVector(heap->elements) - 1;
    void **elements = storageBlockOfVector(heap->elements);
    while (position > 0) {
        /* Dopóki kopiec nie jest poprawny, wartość jest przepychana w górę kopca. */
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

    /* Zamieniany jest najmniejszy element z ostatnim. */
    void *minimum = elements[0];
    swap(elements, 0, elementCount - 1);
    popFromVector(heap->elements, minimum, NULL);
    elementCount--;

    size_t position = 0;
    while (position < elementCount) {
        /* Dopóki kopiec nie jest poprawny, zamieniony element jest spychany w dół. */
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