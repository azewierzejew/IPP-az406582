#include "map_basics.h"


City *otherRoadEnd(Road *road, City *end) {
    if (road == NULL) {
        return NULL;
    }

    if (road->end1 == end) {
        return road->end2;
    }
    if (road->end2 == end) {
        return road->end1;
    }
    return NULL;
}

void doNothing(__attribute__((unused)) void *arg) {}