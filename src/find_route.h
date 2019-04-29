#ifndef DROGI_FIND_ROUTE_H
#define DROGI_FIND_ROUTE_H

#include "map_basics.h"

/**
 * Struktura przechowująca łączny dystans dla drogi.
 */
typedef struct DistanceStruct Distance;

/**
 * Struktura przechowująca wynik szukania drogi.
 */
typedef struct RouteSearchAnswerStruct RouteSearchAnswer;

/**
 * Definicja struktury
 */
struct DistanceStruct {
    uint64_t length;
    int lastRepaired;
};

struct RouteSearchAnswerStruct {
    int count;
    Vector *roads;
    Distance distance;
};

int compareDistances(Distance distance1, Distance distance2);

RouteSearchAnswer findRoute(Map *map, City *city1, City *city2, Vector *usedRoads);

#endif /* DROGI_FIND_ROUTE_H */
