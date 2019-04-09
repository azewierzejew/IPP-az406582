#include "map.h"

#include <stdbool.h>

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

