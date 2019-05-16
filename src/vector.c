#include "vector.h"

#include <stdlib.h>
#include <stdbool.h>


/* Deklaracje struktur. */

/** Przechowuje wektor. */
struct VectorStruct {
    /** Ilość aktualnie zaalokowanego miejsca. */
    size_t space;
    /** Ilość aktualnie wykorzystanego miejsca. */
    size_t count;
    /** Wskaźnik na blok pamięci przetrzymujący wartości. */
    void **holder;
};


/* Funkcje pomocnicze. */


static inline bool resizeVector(Vector *vector, size_t len);

/* Implementacja funkcji pomocniczych. */


static inline bool resizeVector(Vector *vector, size_t len) {
    void **newHolder = realloc(vector->holder, len * sizeof(void *));
    if (newHolder == NULL) {
        return false;
    }

    vector->holder = newHolder;
    vector->space = len;
    return true;
}


/* Funkcje z interfejsu. */


Vector *initVector() {
    Vector *vector = malloc(sizeof(Vector));
    if (vector == NULL) {
        return NULL;
    }

    vector->space = 0;
    vector->count = 0;
    vector->holder = NULL;

    return vector;
}

void deleteVector(Vector *vector, void valueDestructor(void *)) {
    if (vector == NULL) {
        return;
    }

    if (valueDestructor != NULL) {
        for (size_t i = 0; i < vector->count; i++) {
            valueDestructor(vector->holder[i]);
        }
    }
    free(vector->holder);
    free(vector);
}

bool pushToVector(Vector *vector, void *value) {
    if (vector == NULL || value == NULL) {
        return false;
    }

    if (vector->space == vector->count) {
        if (!resizeVector(vector, vector->space * 2 + 2)) {
            return false;
        }
    }

    vector->holder[vector->count++] = value;
    return true;
}

void popFromVector(Vector *vector, void *value, void valueDestructor(void *)) {
    if (vector == NULL || value == NULL) {
        return;
    }

    for (size_t i = vector->count; i > 0;) {
        i--;
        if (vector->holder[i] == value) {
            if (valueDestructor != NULL) {
                valueDestructor(vector->holder[i]);
            }
            vector->holder[i] = vector->holder[--vector->count];
            return;
        }
    }
}

size_t sizeOfVector(Vector *vector) {
    if (vector == NULL) {
        return 0;
    }

    return vector->count;
}

bool isEmptyVector(Vector *vector) {
    if (vector == NULL) {
        return true;
    }

    return vector->count == 0;
}


void **storageBlockOfVector(Vector *vector) {
    if (vector == NULL) {
        return NULL;
    }

    return vector->holder;
}

bool replaceValueWithVector(Vector *vector, void *value, Vector *part) {
    if (!prepareForReplacingValueWithVector(vector, value, part)) {
        return false;
    }

    /* Liczone dla indeksu plus jeden, żeby nie przekręcić wartości. */
    size_t index = vector->count;
    while (index > 0) {
        if (vector->holder[index - 1] == value) {
            break;
        }

        index--;
    }

    if (index == 0) {
        return false;
    }
    index--;

    size_t addedCount = sizeOfVector(part);
    for (size_t i = vector->count - 1; i > index; i--) {
        vector->holder[i - 1 + addedCount] = vector->holder[i];
    }

    for (size_t i = 0; i < addedCount; i++) {
        /* Jeżeli [part == NULL] albo [part->holder == NULL] to tu nie wejdziemy. */
        vector->holder[index + i] = part->holder[i];
    }

    size_t totalCount = vector->count + addedCount - 1;
    vector->count = totalCount;
    deleteVector(part, NULL);
    return true;
}

bool prepareForReplacingValueWithVector(Vector *vector, void *value, Vector *part) {
    if (vector == NULL || isEmptyVector(vector)) {
        return false;
    }

    /* Liczone dla indeksu plus jeden, żeby nie przekręcić wartości. */
    size_t index = vector->count;
    while (index > 0) {
        if (vector->holder[index - 1] == value) {
            break;
        }

        index--;
    }

    if (index == 0) {
        return false;
    }

    if (isEmptyVector(part)) {
        return true;
    }

    size_t addedCount = sizeOfVector(part);
    size_t totalCount = vector->count + addedCount - 1;
    if (totalCount > vector->space) {
        size_t newSize = totalCount;
        if (newSize <= vector->space * 2) {
            newSize = vector->space * 2 + 1;
        }
        if (!resizeVector(vector, newSize)) {
            return false;
        }
    }

    return true;
}

bool existsInVector(Vector *vector, void *value) {
    if (vector == NULL) {
        return false;
    }

    for (size_t i = 0; i < vector->count; i++) {
        if (vector->holder[i] == value) {
            return true;
        }
    }

    return false;
}

bool appendVector(Vector *vector, Vector *part) {
    if (vector == NULL || part == NULL) {
        return false;
    }

    size_t elementCount = vector->count;
    size_t addedCount = sizeOfVector(part);
    size_t totalCount = elementCount + addedCount;
    if (totalCount > vector->space) {
        size_t newSize = totalCount;
        if (newSize <= vector->space * 2) {
            newSize = vector->space * 2 + 1;
        }
        if (!resizeVector(vector, newSize)) {
            return false;
        }
    }

    for (size_t i = 0; i < addedCount; i++) {
        /* Jeśli [part == NULL] lub [part->holder == NULL] to tu nie wejdziemy. */
        vector->holder[elementCount + i] = part->holder[i];
    }

    vector->count = totalCount;
    deleteVector(part, NULL);
    return true;
}