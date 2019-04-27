#include "map.h"
#include "vector.h"
#include "list.h"
#include "dict.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


// Definicje typów.

typedef struct RoadStruct *Road;
typedef struct CityStruct *City;
typedef struct RouteStruct *Route;
typedef char *String;

// Deklaracje struktur.

struct RoadStruct {
    int lastRepaired;
    unsigned length;
    City end1, end2;
};


struct CityStruct {
    String name;
    Vector roads;
};


struct RouteStruct {
    City end1, end2;
    List roads;
};

struct Map {
    Dict cities;
    Vector routes;
};


// Funkcje pomocnicze.

static City initCity(const char *name);

static Road initRoad(int builtYear, unsigned length, City end1, City end2);

static Route initRoute(List roads, City end1, City end2);

static void doNothing(void *arg);

static void deleteRoad(void *roadVoid);

static void deleteCity(void *cityVoid);

static void deleteRoute(void *routeVoid);

static bool correctChar(char a);

static bool checkCityName(const char *name);

static City otherRoadEnd(Road road, City end);


// Implementacja funkcji pomocniczych.

static City initCity(const char *name) {
    City city = malloc(sizeof(struct CityStruct));
    if (city == NULL) {
        return NULL;
    }

    city->name = malloc(sizeof(char) * (strlen(name) + 1));
    if (city->name == NULL) {
        free(city);
        return NULL;
    }
    strcpy(city->name, name);
    return city;
}

static Road initRoad(int builtYear, unsigned length, City end1, City end2) {
    Road road = malloc(sizeof(struct RoadStruct));
    if (road == NULL) {
        return NULL;
    }

    road->lastRepaired = builtYear;
    road->length = length;
    road->end1 = end1;
    road->end2 = end2;
    return road;
}

static Route initRoute(List roads, City end1, City end2) {
    Route route = malloc(sizeof(struct RouteStruct));
    if (route == NULL) {
        return NULL;
    }

    route->roads = roads;
    route->end1 = end1;
    route->end2 = end2;
    return route;
}

static void doNothing(__attribute__((unused)) void *arg) {

}

static void deleteRoad(void *roadVoid) {
    Road road = roadVoid;
    if (road == NULL) {
        return;
    }

    // Droga musi być usunięta z obu końców i nie ma roku 0, więc rok naprawy 0 oznacza połowiczne usunięcie.
    if (road->lastRepaired == 0) {
        free(road);
    } else {
        road->lastRepaired = 0;
    }
}

static void deleteCity(void *cityVoid) {
    City city = cityVoid;
    if (city == NULL) {
        return;
    }

    free(city->name);
    deleteVector(city->roads, deleteRoad);
    free(city);
}

static void deleteRoute(void *routeVoid) {
    Route route = routeVoid;
    if (route == NULL) {
        return;
    }

    deleteList(route->roads, doNothing);
    free(route);
}

static bool correctChar(char a) {
    return !(a >= 0 && a <= 31);
}

static bool checkCityName(const char *name) {
    if (name == NULL) {
        return false;
    }

    size_t len = strlen(name);
    if (len == 0) {
        return false;
    }

    for (size_t i = 0; i < len; i++) {
        if (!correctChar(name[i])) {
            return false;
        }
    }

    return true;
}

static City otherRoadEnd(Road road, City end) {
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


// Funkcje z interfejsu.

Map *newMap() {
    Map *map = malloc(sizeof(Map));
    if (map == NULL) {
        return NULL;
    }

    map->cities = initDict();
    map->routes = initVector();

    if (map->cities == NULL || map->routes == NULL) {
        deleteMap(map);
        return NULL;
    }

    return map;
}

void deleteMap(Map *map) {
    if (map == NULL) {
        return;
    }

    deleteDict(map->cities, deleteCity);
    deleteVector(map->routes, deleteRoute);
    free(map);
}

bool addRoad(Map *map, const char *cityName1, const char *cityName2,
             unsigned length, int builtYear) {
    if (map == NULL || builtYear == 0) {
        return false;
    }

    if (!checkCityName(cityName1) || !checkCityName(cityName2)) {
        return false;
    }

    City city1 = valueInDict(map->cities, cityName1);
    if (city1 == NULL) {
        city1 = initCity(cityName1);
        if (city1 == NULL) {
            return false;
        }

        if (!addToDict(map->cities, cityName1, city1)) {
            return false;
        }
    }

    City city2 = valueInDict(map->cities, cityName2);
    if (city2 == NULL) {
        city2 = initCity(cityName2);
        if (city2 == NULL) {
            return false;
        }

        if (!addToDict(map->cities, cityName2, city2)) {
            return false;
        }
    }

    Road road = initRoad(builtYear, length, city1, city2);
    if (road == NULL) {
        return false;
    }

    if (!pushToVector(city1->roads, road)) {
        return false;
    }

    if (!pushToVector(city2->roads, road)) {
        popFromVector(city1->roads, road, free);
        return false;
    }

    return true;
}


bool repairRoad(Map *map, const char *cityName1, const char *cityName2, int repairYear) {
    if (map == NULL || repairYear == 0) {
        return false;
    }

    if (!checkCityName(cityName1) || !checkCityName(cityName2)) {
        return false;
    }

    City city1 = valueInDict(map->cities, cityName1);
    if (city1 == NULL) {
        city1 = initCity(cityName1);
        if (city1 == NULL) {
            return false;
        }

        if (!addToDict(map->cities, cityName1, city1)) {
            free(city1);
            return false;
        }
    }

    City city2 = valueInDict(map->cities, cityName2);
    if (city2 == NULL) {
        city2 = initCity(cityName1);
        if (city2 == NULL) {
            return false;
        }

        if (!addToDict(map->cities, cityName2, city2)) {
            free(city2);
            return false;
        }
    }

    Road road = NULL;

    size_t len = sizeOfVector(city1->roads);
    // Dla wydajności zamieniane są miasta jeśli z drugiego wychodzi mniej odcinków.
    {
        size_t len2 = sizeOfVector(city2->roads);
        if (len > len2) {
            City tmp = city1;
            city1 = city2;
            city2 = tmp;
            len = len2;
        }

    }

    Road *roads = (Road *) arrayFromVector(city1->roads);
    for (size_t i = 0; i < len && road == NULL; i++) {
        if (city2 == otherRoadEnd(roads[i], city1)) {
            road = roads[i];
        }
    }

    if (road == NULL) {
        return false;
    }

    if (road->lastRepaired > repairYear) {
        return false;
    }

    road->lastRepaired = repairYear;
    return true;
}


bool newRoute(Map *map, unsigned routeId, const char *cityName1, const char *cityName2) {

}

bool extendRoute(Map *map, unsigned routeId, const char *cityName) {

}

bool removeRoad(Map *map, const char *cityName1, const char *cityName2) {

}


char const *getRouteDescription(Map *map, unsigned routeId) {

}

