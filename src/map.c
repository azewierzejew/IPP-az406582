#include "map.h"
#include "vector.h"
#include "dict.h"
#include "map_basics.h"
#include "find_route.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <inttypes.h>


/* Stałe globalne. */

static const unsigned MAX_ROUTE_ID = 999;
static const size_t MAX_LENGTH_LENGTH = 10;
static const size_t MAX_YEAR_LENGTH = 11;
static const size_t MAX_ROUTE_ID_LENGTH = 3;


/* Funkcje pomocnicze. */

static City *initCity(const char *name, size_t id);

static Road *initRoad(int builtYear, unsigned length, City *end1, City *end2);

static Route *initRoute(Vector *roads, City *end1, City *end2);

static void deleteRoad(void *roadVoid);

static void deleteCity(void *cityVoid);

static void deleteRoute(void *routeVoid);

static bool correctChar(char a);

static bool checkName(const char *name);

static Road *findRoad(City *city1, City *city2);

static City *addCity(Map *map, const char *cityName);

static void addNameToDescription(char **description, char *name);

static void addUnsignedToDescription(char **description, unsigned number);

static void addIntToDescription(char **description, int number);

static bool checkRouteOrientation(Route *route, City *city1, City *city2);


/* Implementacja funkcji pomocniczych. */

static City *initCity(const char *name, size_t id) {
    City *city = malloc(sizeof(City));
    if (city == NULL) {
        return NULL;
    }

    city->name = malloc(sizeof(char) * (strlen(name) + 1));
    if (city->name == NULL) {
        free(city);
        return NULL;
    }

    strcpy(city->name, name);
    city->roads = initVector();

    if (city->roads == NULL) {
        free(city->name);
        free(city);
        return NULL;
    }

    city->id = id;
    return city;
}

static Road *initRoad(int builtYear, unsigned length, City *end1, City *end2) {
    Road *road = malloc(sizeof(Road));
    if (road == NULL) {
        return NULL;
    }

    road->lastRepaired = builtYear;
    road->length = length;
    road->end1 = end1;
    road->end2 = end2;
    return road;
}

static Route *initRoute(Vector *roads, City *end1, City *end2) {
    Route *route = malloc(sizeof(Route));
    if (route == NULL) {
        return NULL;
    }

    route->roads = roads;
    route->end1 = end1;
    route->end2 = end2;
    return route;
}

static void deleteRoad(void *roadVoid) {
    Road *road = roadVoid;
    if (road == NULL) {
        return;
    }

    /* Droga musi być usunięta z obu końców i nie ma roku 0,
     * więc rok naprawy 0 oznacza połowiczne usunięcie. */
    if (road->lastRepaired == 0) {
        free(road);
    } else {
        road->lastRepaired = 0;
    }
}

static void deleteCity(void *cityVoid) {
    City *city = cityVoid;
    if (city == NULL) {
        return;
    }

    free(city->name);
    deleteVector(city->roads, deleteRoad);
    free(city);
}

static void deleteRoute(void *routeVoid) {
    Route *route = routeVoid;
    if (route == NULL) {
        return;
    }

    deleteVector(route->roads, doNothing);
    free(route);
}

static bool correctChar(char a) {
    return !(a >= 0 && a <= 31);
}

static bool checkName(const char *name) {
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

static Road *findRoad(City *city1, City *city2) {
    if (city1 == NULL || city2 == NULL) {
        return NULL;
    }

    size_t len = sizeOfVector(city1->roads);

    /* Dla wydajności zamieniane są miasta jeśli z drugiego wychodzi mniej odcinków. */
    {
        size_t len2 = sizeOfVector(city2->roads);
        if (len > len2) {
            City *tmp = city1;
            city1 = city2;
            city2 = tmp;
            len = len2;
        }
    }

    Road *road = NULL;
    Road **roads = (Road **) storageBlockOfVector(city1->roads);
    for (size_t i = 0; i < len && road == NULL; i++) {
        if (city2 == otherRoadEnd(roads[i], city1)) {
            road = roads[i];
        }
    }
    return road;
}

static City *addCity(Map *map, const char *cityName) {
    if (map == NULL) {
        return NULL;
    }

    City *city = initCity(cityName, map->cityCount);
    if (city == NULL) {
        return NULL;
    }

    if (!addToDict(map->cities, cityName, city)) {
        deleteCity(city);
        return NULL;
    }

    map->cityCount++;
    return city;
}

static void addNameToDescription(char **description, char *name) {
    if (description == NULL || name == NULL) {
        return;
    }

    strcat(*description, name);
    *description += strlen(name);
    strcat(*description, ";");
    *description += 1;
}

static void addUnsignedToDescription(char **description, unsigned number) {
    sprintf(*description, "%u;", number);
    *description += strlen(*description);
}

static void addIntToDescription(char **description, int number) {
    sprintf(*description, "%d;", number);
    *description += strlen(*description);
}

static bool checkRouteOrientation(Route *route, City *city1, City *city2) {
    if (route == NULL) {
        return false;
    }

    if (city1 == city2) {
        return false;
    }

    City *cityPair[2] = {city1, city2};
    size_t correctCount = 0;

    size_t roadCount = sizeOfVector(route->roads);
    Road **roads = (Road **) storageBlockOfVector(route->roads);

    City *position = route->end1;
    for (size_t i = 0; i < roadCount; i++) {
        if (position == cityPair[correctCount]) {
            correctCount++;
            if (correctCount == 2) {
                return true;
            }
        }
        position = otherRoadEnd(roads[i], position);
    }
    if (position == cityPair[correctCount]) {
        correctCount++;
        if (correctCount == 2) {
            return true;
        }
    }

    return false;
}


/* Funkcje z interfejsu. */

Map *newMap() {
    Map *map = malloc(sizeof(Map));
    if (map == NULL) {
        return NULL;
    }

    map->cities = initDict();
    map->routes = calloc(MAX_ROUTE_ID + 1, sizeof(Route));
    map->cityCount = 0;
    map->doneRoutes = initVector();
    if (map->cities == NULL || map->routes == NULL || map->doneRoutes == NULL) {
        deleteMap(map);
        return NULL;
    }
    return map;
}

void deleteMap(Map *map) {
    if (map == NULL) { return; }
    for (size_t i = 0; i <= MAX_ROUTE_ID; i++) {
        deleteRoute(map->routes[i]);
    }

    deleteDict(map->cities, deleteCity);
    free(map->routes);
    free(map);
}

bool addRoad(Map *map, const char *cityName1, const char *cityName2,
             unsigned length, int builtYear) {
    if (map == NULL || builtYear == 0 || length == 0) {
        return false;
    }

    if (!checkName(cityName1) || !checkName(cityName2) || strcmp(cityName1, cityName2) == 0) {
        return false;
    }

    City *city1 = valueInDict(map->cities, cityName1);
    if (city1 == NULL) {
        city1 = addCity(map, cityName1);
        if (city1 == NULL) { return false; }
    }

    City *city2 = valueInDict(map->cities, cityName2);
    if (city2 == NULL) {
        city2 = addCity(map, cityName2);
        if (city2 == NULL) { return false; }
    }

    if (findRoad(city1, city2) != NULL) {
        return false;
    }

    Road *road = initRoad(builtYear, length, city1, city2);
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

    if (!checkName(cityName1) || !checkName(cityName2) || strcmp(cityName1, cityName2) == 0) {
        return false;
    }

    City *city1 = valueInDict(map->cities, cityName1);
    if (city1 == NULL) {
        return false;
    }

    City *city2 = valueInDict(map->cities, cityName2);
    if (city2 == NULL) {
        return false;
    }

    Road *road = findRoad(city1, city2);
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
    if (map == NULL || routeId == 0 || routeId > MAX_ROUTE_ID ||
        map->routes[routeId] != NULL) {
        return false;
    }

    if (!checkName(cityName1) || !checkName(cityName2) || strcmp(cityName1, cityName2) == 0) {
        return false;
    }

    City *city1 = valueInDict(map->cities, cityName1);
    if (city1 == NULL) {
        return false;
    }

    City *city2 = valueInDict(map->cities, cityName2);
    if (city2 == NULL) {
        return false;
    }

    Vector *roads = findRoute(map, city1, city2, NULL).roads;
    if (roads == NULL) {
        return false;
    }

    Route *route = initRoute(roads, city1, city2);
    if (route == NULL) {
        deleteVector(roads, doNothing);
        return false;
    }

    unsigned *id = malloc(sizeof(unsigned));
    if (id == NULL) {
        deleteVector(roads, doNothing);
        return false;
    }

    *id = routeId;
    if (!pushToVector(map->doneRoutes, id)) {
        deleteVector(roads, doNothing);
        return false;
    }
    map->routes[routeId] = route;
    return true;
}

bool extendRoute(Map *map, unsigned routeId, const char *cityName) {
    if (map == NULL || routeId == 0 || routeId > MAX_ROUTE_ID) {
        return false;
    }

    if (!checkName(cityName) || map->routes[routeId] == NULL) {
        return false;
    }

    Route *route = map->routes[routeId];

    City *city = valueInDict(map->cities, cityName);
    if (city == NULL) {
        return false;
    }

    if (route->end1 == city || route->end2 == city) {
        return false;
    }
    {
        size_t usedRoadsCount = sizeOfVector(route->roads);
        Road **usedRoadsArray = (Road **) storageBlockOfVector(route->roads);
        for (size_t i = 0; i < usedRoadsCount; i++) {
            if (usedRoadsArray[i]->end1 == city || usedRoadsArray[i]->end2 == city) {
                return false;
            }
        }
    }

    RouteSearchAnswer answer1 = findRoute(map, city, route->end1, route->roads);
    RouteSearchAnswer answer2 = findRoute(map, route->end2, city, route->roads);

    /* W którymś wyszukiwaniu nastąpił błąd. */
    if (answer1.count == -1 || answer2.count == -1) {
        deleteVector(answer1.roads, doNothing);
        deleteVector(answer2.roads, doNothing);
        return false;
    }

    /* Żadne wyszukiwanie nie znalazło dokładnie jednego wyniku. */
    if (answer1.count != 1 && answer2.count != 1) {
        deleteVector(answer1.roads, doNothing);
        deleteVector(answer2.roads, doNothing);
        return false;
    }

    bool connectToEnd1;
    if (answer1.count == 1) {
        if (answer2.count == 0) {
            connectToEnd1 = true;
        } else if (answer2.count == 1) {
            int comparison = compareDistances(answer1.distance, answer2.distance);

            if (comparison < 0) {
                connectToEnd1 = true;
            } else if (comparison > 0) {
                connectToEnd1 = false;
            } else {
                deleteVector(answer1.roads, doNothing);
                deleteVector(answer2.roads, doNothing);
                return false;
            }
        } else {
            /* Więcej niż jeden wynik w drugim szukaniu,
             * więc da się tylko gdy w pierwszym jest ściśle lepszy dystans. */
            int comparison = compareDistances(answer1.distance, answer2.distance);

            if (comparison < 0) {
                connectToEnd1 = true;
            } else {
                deleteVector(answer1.roads, doNothing);
                deleteVector(answer2.roads, doNothing);
                return false;
            }
        }
    } else { /* [answer1.count != 1] czyli [answer2.count == 1]. */
        if (answer1.count == 0) {
            connectToEnd1 = false;
        } else {
            /* Więcej niż jeden wynik w pierwszym szukaniu,
             * więc da się tylko gdy w drugim jest ściśle lepszy dystans. */
            int comparison = compareDistances(answer1.distance, answer2.distance);

            if (comparison > 0) {
                connectToEnd1 = false;
            } else {
                deleteVector(answer1.roads, doNothing);
                deleteVector(answer2.roads, doNothing);
                return false;
            }
        }
    }

    Vector *roads1 = answer1.roads;
    Vector *roads2 = answer2.roads;
    if (connectToEnd1) {
        deleteVector(roads2, doNothing);
        size_t oldRoadCount = sizeOfVector(route->roads);
        Road **oldRoads = (Road **) storageBlockOfVector(route->roads);

        for (size_t i = 0; i < oldRoadCount; i++) {
            if (!pushToVector(roads1, oldRoads[i])) {
                deleteVector(roads1, doNothing);
                return false;
            }
        }

        deleteVector(route->roads, doNothing);
        route->roads = roads1;
        route->end1 = city;
    } else {
        deleteVector(roads1, doNothing);

        if (!pushToVector(route->roads, NULL)) {
            deleteVector(roads2, doNothing);
            return false;
        }

        if (!replaceValueWithVector(route->roads, NULL, roads2)) {
            deleteVector(roads2, doNothing);
            popFromVector(route->roads, NULL, doNothing);
            return false;
        }
        route->end2 = city;
    }

    return true;
}

bool removeRoad(Map *map, const char *cityName1, const char *cityName2) {
    if (map == NULL) {
        return false;
    }

    if (!checkName(cityName1) || !checkName(cityName2) || strcmp(cityName1, cityName2) == 0) {
        return false;
    }

    City *city1 = valueInDict(map->cities, cityName1);
    if (city1 == NULL) {
        return false;
    }

    City *city2 = valueInDict(map->cities, cityName2);
    if (city2 == NULL) {
        return false;
    }

    Road *road = findRoad(city1, city2);
    if (road == NULL) {
        return false;
    }

    /* Przeszukiwanie grafu nie będzie mogło użyć tego odcinka. */
    unsigned oldLength = road->length;
    road->length = 0;

    Vector **replacementParts = calloc(MAX_ROUTE_ID + 1, sizeof(Vector *));
    if (replacementParts == NULL) {
        road->length = oldLength;
        return false;
    }

    size_t doneRouteCount = sizeOfVector(map->doneRoutes);
    unsigned **doneRoutes = (unsigned **) storageBlockOfVector(map->doneRoutes);
    for (size_t i = 0; i < doneRouteCount; i++) {
        unsigned id = *doneRoutes[i];
        Route *route = map->routes[id];
        if (route == NULL) {
            continue;
        }

        if (!existsInVector(route->roads, road)) {
            continue;
        }

        if (checkRouteOrientation(route, city1, city2)) {
            replacementParts[id] = findRoute(map, city1, city2, route->roads).roads;
        } else {
            replacementParts[id] = findRoute(map, city2, city1, route->roads).roads;
        }
        if (replacementParts[id] == NULL ||
            !prepareForReplacingValueWithVector(route->roads, road, replacementParts[id])) {
            for (size_t j = 0; j <= id; j++) {
                deleteVector(replacementParts[j], doNothing);
            }
            free(replacementParts);
            road->length = oldLength;
            return false;
        }
    }


    for (size_t i = 0; i < doneRouteCount; i++) {
        unsigned id = *doneRoutes[i];
        if (map->routes[id] != NULL && replacementParts[id] != NULL) {
            // Jest pewność, że się powiedzie.
            replaceValueWithVector(map->routes[id]->roads, road, replacementParts[id]);
        }
    }

    popFromVector(city1->roads, road, doNothing);
    popFromVector(city2->roads, road, doNothing);
    free(replacementParts);
    free(road);
    return true;
}

char const *getRouteDescription(Map *map, unsigned routeId) {
    if (map == NULL || routeId == 0 || routeId > MAX_ROUTE_ID || map->routes[routeId] == NULL) {
        char *description = malloc(1);
        if (description == NULL) {
            return NULL;
        }
        description[0] = '\0';
        return description;
    }

    Route *route = map->routes[routeId];
    size_t roadCount = sizeOfVector(route->roads);
    Road **roads = (Road **) storageBlockOfVector(route->roads);

    City *position = route->end1;
    size_t totalLength = MAX_ROUTE_ID_LENGTH + 1;
    for (size_t i = 0; i < roadCount; i++) {
        totalLength += strlen(position->name + 1);
        totalLength += MAX_LENGTH_LENGTH + 1;
        totalLength += MAX_YEAR_LENGTH + 1;
        position = otherRoadEnd(roads[i], position);
    }
    totalLength += strlen(position->name) + 1;

    char *description = malloc(sizeof(char) * totalLength);
    if (description == NULL) {
        return NULL;
    }

    description[0] = '\0';
    position = route->end1;
    char *decriptionPosition = description;
    addUnsignedToDescription(&decriptionPosition, routeId);
    for (size_t i = 0; i < roadCount; i++) {
        addNameToDescription(&decriptionPosition, position->name);
        addUnsignedToDescription(&decriptionPosition, roads[i]->length);
        addIntToDescription(&decriptionPosition, roads[i]->lastRepaired);
        position = otherRoadEnd(roads[i], position);
    }
    strcat(decriptionPosition, position->name);

    return description;
}