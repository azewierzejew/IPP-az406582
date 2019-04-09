#ifndef DROGI_SET_H
#define DROGI_SET_H

#include <stdbool.h>

typedef struct Set Set;

Set *initSet();

bool addToSet(Set set, const void *value);

bool isEmptySet(Set set);

void removeFromSet(Set set, const void *value);

void deleteSet(Set set);

#endif //DROGI_SET_H
