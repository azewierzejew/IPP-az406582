#ifndef DROGI_VECTOR_H
#define DROGI_VECTOR_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct VectorStruct Vector;

Vector *initVector();

void deleteVector(Vector *vector, void valueDestructor(void *));

bool pushToVector(Vector *vector, void *value);

void popFromVector(Vector *vector, void *value, void valueDestructor(void *));

size_t sizeOfVector(Vector *vector);

bool isEmptyVector(Vector *vector);

void **storageBlockOfVector(Vector *vector);

bool replaceValueWithVector(Vector *vector, void *value, Vector *part);

bool prepareForReplacingValueWithVector(Vector *vector, void *value, Vector *part);

bool existsInVector(Vector *vector, void *value);

#endif //DROGI_VECTOR_H
