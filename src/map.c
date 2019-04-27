#include "map.h"
#include "set.h"
#include "list.h"
#include "dict.h"

#include <stdbool.h>
#include <stdlib.h>

typedef struct RoadStruct *Road;
typedef struct CityStruct *City;
typedef struct RouteStruct *Route;
typedef char *String;

struct RoadStruct {
    int lastRepaired;
    unsigned length;
    City end1, end2;
};


struct CityStruct {
    String name;
    Set roads;
};


struct RouteStruct {
    City end1, end2;
    List roads;
};

struct Map {
    Dict cities;
    Route routes[1000];
};


Map *newMap() {
    Map *map = malloc(sizeof(Map));
    if (map == NULL) {
        return NULL;
    }
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

