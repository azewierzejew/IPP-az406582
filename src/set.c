#include "set.h"

#include <stdlib.h>

struct SetS {
    size_t space, count;
    void **holder;
};

static inline bool resizeSet(Set set, const size_t len) {
    void **newHolder = realloc(set.holder, len * sizeof(void *));
    if (newHolder == NULL) {
        return NULL;
    }

    set.holder = newHolder;
    set.space = len;
    return true;
}

Set *initSet() {
    Set *setPtr = malloc(sizeof(Set));
    if (setPtr == NULL) {
        return NULL;
    }

    setPtr->space = 0;
    setPtr->count = 0;
    setPtr->holder = NULL;

    return setPtr;
}

bool addToSet(Set set, const void *value) {
    if (set.space == set.count) {
        if (!resizeSet(set, set.space * 2 + 1)) {
            return false;
        }
    }

    set.holder[set.count++] = value;
    return true;
}

bool isEmptySet(Set set) {
    return set.count == 0;
}

void removeFromSet(Set set, const void *value) {
    for (int i = 0; i < set.count; i++) {
        if (set.holder[i] == value) {
            set.holder[i] = set.holder[--set.count];
            return;
        }
    }
}

void deleteSet(Set set) {
    free(set.holder);
    free(&set);
}