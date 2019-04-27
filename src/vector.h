#ifndef DROGI_VECTOR_H
#define DROGI_VECTOR_H

#include <stdbool.h>
#include <stdlib.h>

struct VectorStruct;

typedef struct VectorStruct *Vector;

Vector initVector();

void deleteVector(Vector vector, void valueDestructor(void *));

bool isEmptyVector(Vector vector);

bool addToVector(Vector vector, void *value);

void removeFromVector(Vector vector, void *value, void valueDestructor(void *));

size_t sizeOfVector(Vector vector);

void **arrayFromVector(Vector vector);

#endif //DROGI_VECTOR_H
