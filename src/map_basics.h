#ifndef DROGI_MAP_BASICS_H
#define DROGI_MAP_BASICS_H


#include "vector.h"
#include "dict.h"

#include <inttypes.h>


/* Definicje typów. */

typedef struct Map Map;
typedef struct RoadStruct Road;
typedef struct CityStruct City;
typedef struct RouteStruct Route;


/* Deklaracje struktur. */

struct RoadStruct {
    int lastRepaired;
    City *end1;
    City *end2;
    unsigned length;
};

struct CityStruct {
    char *name;
    Vector *roads;
    size_t id;
};

struct RouteStruct {
    City *end1;
    City *end2;
    Vector *roads;
};

struct Map {
    Dict *cities;
    Route **routes;
    size_t cityCount;
};


City *otherRoadEnd(Road *road, City *end);

void doNothing(void *arg);

#endif //DROGI_MAP_BASICS_H
