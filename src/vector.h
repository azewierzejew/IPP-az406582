#ifndef DROGI_VECTOR_H
#define DROGI_VECTOR_H

#include <stdlib.h>
#include <stdbool.h>

struct VectorStruct;

typedef struct VectorStruct *Vector;

Vector initVector();

void deleteVector(Vector vector, void valueDestructor(void *));

bool isEmptyVector(Vector vector);

bool pushToVector(Vector vector, void *value);

void popFromVector(Vector vector, void *value, void valueDestructor(void *));

size_t sizeOfVector(Vector vector);

void **arrayFromVector(Vector vector);

#endif //DROGI_VECTOR_H
