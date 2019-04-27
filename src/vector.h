#ifndef DROGI_VECTOR_H
#define DROGI_VECTOR_H

#include <stdbool.h>

struct VectorStruct;

typedef struct VectorStruct *Vector;

struct VectorIteratorStruct;

typedef struct VectorIteratorStruct *VectorIterator;

Vector initVector();

void deleteVector(Vector vector, void valueDestructor(void *));

bool isEmptyVector(Vector vector);

bool addToVector(Vector vector, void *value);

void removeFromVector(Vector vector, void *value, void valueDestructor(void *));

VectorIterator iterateVector(Vector vector);

void *getVectorIteratorNext(VectorIterator iterator);

#endif //DROGI_VECTOR_H
