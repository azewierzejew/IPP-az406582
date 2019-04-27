#include "vector.h"

#include <stdlib.h>

struct VectorStruct {
    size_t space, count;
    void **holder;
};

static inline bool resizeVector(Vector vector, const size_t len) {
    void **newHolder = realloc(vector->holder, len * sizeof(void *));
    if (newHolder == NULL) {
        return NULL;
    }

    vector->holder = newHolder;
    vector->space = len;
    return true;
}

Vector initVector() {
    Vector vector = malloc(sizeof(struct VectorStruct));
    if (vector == NULL) {
        return NULL;
    }

    vector->space = 0;
    vector->count = 0;
    vector->holder = NULL;

    return vector;
}

void deleteVector(Vector vector, void valueDestructor(void *)) {
    if (vector == NULL) {
        return;
    }

    for (size_t i = 0; i < vector->count; i++) {
        valueDestructor(vector->holder[i]);
    }
    free(vector->holder);
    free(vector);
}

bool isEmptyVector(Vector vector) {
    if (vector == NULL) {
        return false;
    }

    return vector->count == 0;
}

bool addToVector(Vector vector, void *value) {
    if (vector == NULL) {
        return false;
    }

    if (vector->space == vector->count) {
        if (!resizeVector(vector, vector->space * 2 + 1)) {
            return false;
        }
    }

    vector->holder[vector->count++] = value;
    return true;
}

void removeFromVector(Vector vector, void *value, void valueDestructor(void *)) {
    if (vector == NULL) {
        return;
    }

    for (int i = 0; i < vector->count; i++) {
        if (vector->holder[i] == value) {
            valueDestructor(vector->holder[i]);
            vector->holder[i] = vector->holder[--vector->count];
            return;
        }
    }
}

size_t sizeOfVector(Vector vector) {
    if (vector == NULL) {
        return 0;
    }

    return vector->count;
}

void **arrayFromVector(Vector vector) {
    if (vector == NULL) {
        return NULL;
    }

    return vector->holder;
}