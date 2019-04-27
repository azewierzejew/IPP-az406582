#ifndef DROGI_SET_H
#define DROGI_SET_H

#include <stdbool.h>

struct SetStruct;

typedef struct SetStruct *Set;

struct SetIteratorStruct;

typedef struct SetIteratorStruct *SetIterator;

Set initSet();

void deleteSet(Set set, void valueDestructor(void *));

bool isEmptySet(Set set);

bool addToSet(Set set, void *value);

void removeFromSet(Set set, void *value, void valueDestructor(void *));

SetIterator iterateSet(Set set);

void *getSetIteratorNext(SetIterator iterator);

#endif //DROGI_SET_H
