#include "set.h"

#include <stdlib.h>

struct SetStruct {
    size_t space, count;
    void **holder;
};

struct SetIteratorStruct {
    Set set;
    size_t number;
};

static inline bool resizeSet(Set set, const size_t len) {
    void **newHolder = realloc(set->holder, len * sizeof(void *));
    if (newHolder == NULL) {
        return NULL;
    }

    set->holder = newHolder;
    set->space = len;
    return true;
}

Set initSet() {
    Set set = malloc(sizeof(struct SetStruct));
    if (set == NULL) {
        return NULL;
    }

    set->space = 0;
    set->count = 0;
    set->holder = NULL;

    return set;
}

void deleteSet(Set set, void valueDestructor(void *)) {
    if (set == NULL) {
        return;
    }

    for (size_t i = 0; i < set->count; i++) {
        valueDestructor(set->holder[i]);
    }
    free(set->holder);
    free(set);
}

bool isEmptySet(Set set) {
    if (set == NULL) {
        return false;
    }

    return set->count == 0;
}

bool addToSet(Set set, void *value) {
    if (set == NULL) {
        return false;
    }

    if (set->space == set->count) {
        if (!resizeSet(set, set->space * 2 + 1)) {
            return false;
        }
    }

    set->holder[set->count++] = value;
    return true;
}

void removeFromSet(Set set, void *value, void valueDestructor(void *)) {
    if (set == NULL) {
        return;
    }

    for (int i = 0; i < set->count; i++) {
        if (set->holder[i] == value) {
            valueDestructor(set->holder[i]);
            set->holder[i] = set->holder[--set->count];
            return;
        }
    }
}

SetIterator iterateSet(Set set) {
    if (set == NULL) {
        return NULL;
    }

    SetIterator iterator = malloc(sizeof(struct SetIteratorStruct));
    if (iterator == NULL) {
        return NULL;
    }

    iterator->set = set;
    iterator->number = 0;
    return iterator;
}

void *getSetIteratorNext(SetIterator iterator) {
    if (iterator == NULL) {
        return NULL;
    }

    if (iterator->number >= iterator->set->count) {
        return NULL;
    }

    return iterator->set->holder[iterator->number++];
}