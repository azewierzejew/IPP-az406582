#include "map.h"
#include "set.h"

#include <stdbool.h>

typedef struct Road Road;

struct Road {
    int lastRepaired;
    int length;
};

typedef struct City City;

struct City {

};

typedef struct Route Route;

struct Route {
    City end1, end2;
    Set roads1;
};

struct Map {

};


Map *newMap() {

}

void deleteMap(Map *map) {

}

bool addRoad(Map *map, const char *city1, const char *city2, unsigned length, int builtYear) {

}


bool repairRoad(Map *map, const char *city1, const char *city2, int repairYear) {

}


bool newRoute(Map *map, unsigned routeId, const char *city1, const char *city2) {

}

bool extendRoute(Map *map, unsigned routeId, const char *city) {

}

bool removeRoad(Map *map, const char *city1, const char *city2) {

}


char const *getRouteDescription(Map *map, unsigned routeId) {

}

