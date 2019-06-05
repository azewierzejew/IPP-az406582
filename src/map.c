#define _GNU_SOURCE

#include "map.h"
#include "map_types.h"
#include "map_checkers.h"
#include "map_graph.h"
#include "map_route.h"
#include "map_find_route.h"

#include "vector.h"
#include "dict.h"
#include "utility.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <inttypes.h>


/* Stałe globalne. */

static const size_t MAX_LENGTH_LENGTH = 10;
static const size_t MAX_YEAR_LENGTH = 11;
static const size_t MAX_ROUTE_ID_LENGTH = 3;


/* Funkcje pomocnicze. */

static int compareSize_t(const void *aPtr, const void *bPtr);

static bool checkForDuplicateIds(size_t *ids, size_t idCount);

static City *addCity(Map *map, const char *cityName);

static void addNameToDescription(char **description, const char *name);

static void addUnsignedToDescription(char **description, unsigned number);

static void addIntToDescription(char **description, int number);


/* Implementacja funkcji pomocniczych. */

static int compareSize_t(const void *aPtr, const void *bPtr) {
    if (aPtr == NULL && bPtr == NULL) {
        return 0;
    }
    if (aPtr == NULL) {
        return -1;
    }
    if (bPtr == NULL) {
        return 1;
    }

    size_t a = *(size_t *) aPtr;
    size_t b = *(size_t *) bPtr;
    if (a < b) {
        return -1;
    }
    if (a > b) {
        return 1;
    }
    return 0;
}

static bool checkForDuplicateIds(size_t *ids, size_t idCount) {
    if (ids == NULL && idCount == 0) {
        return true;
    }
    if (ids == NULL) {
        return false;
    }
    if (idCount <= 1) {
        return true;
    }

    /* W posortowanym ciągu dwa takie same id będą obok siebie. */
    qsort(ids, idCount, sizeof(size_t), compareSize_t);
    for (size_t i = 0; i < idCount - 1; i++) {
        if (ids[i] == ids[i + 1]) {
            return false;
        }
    }
    return true;
}

static City *addCity(Map *map, const char *cityName) {
    City *city = NULL;
    FAIL_IF(map == NULL);

    city = initCity(cityName, map->cityCount);
    FAIL_IF(city == NULL || !addToDict(map->cities, cityName, city));
    map->cityCount++;
    return city;

    FAILURE:

    deleteCity(city);
    return NULL;
}

static void addNameToDescription(char **description, const char *name) {
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


/* Funkcje z interfejsu. */

Map *newMap() {
    Map *map = malloc(sizeof(Map));
    if (map == NULL) {
        return NULL;
    }

    map->cities = initDict();
    map->routes = calloc(MAX_ROUTE_ID + 1, sizeof(Route));
    map->cityCount = 0;
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

    if (map->routes != NULL) {
        for (size_t i = 0; i <= MAX_ROUTE_ID; i++) {
            deleteRoute(map->routes[i]);
        }
    }

    deleteDict(map->cities, deleteCity);
    free(map->routes);
    free(map);
}

bool addRoad(Map *map, const char *cityName1, const char *cityName2,
             unsigned length, int builtYear) {
    City *city1 = NULL;
    City *city2 = NULL;
    Road *road = NULL;

    FAIL_IF(map == NULL || builtYear == 0 || length == 0);
    FAIL_IF(!checkName(cityName1) || !checkName(cityName2) || strcmp(cityName1, cityName2) == 0);

    city1 = valueInDict(map->cities, cityName1);
    city2 = valueInDict(map->cities, cityName2);

    city1 = city1 == NULL ? addCity(map, cityName1) : city1;
    city2 = city2 == NULL ? addCity(map, cityName2) : city2;

    FAIL_IF(city1 == NULL || city2 == NULL);
    FAIL_IF(findRoad(city1, city2) != NULL);

    road = initRoad(builtYear, length, city1, city2);
    FAIL_IF(road == NULL);
    FAIL_IF(!pushToVector(city1->roads, road));
    FAIL_IF(!pushToVector(city2->roads, road));

    return true;

    FAILURE:

    if (city1 != NULL) {
        popFromVector(city1->roads, road, NULL);
    }
    if (city2 != NULL) {
        popFromVector(city2->roads, road, NULL);
    }
    deleteRoad(road);
    return false;
}

bool repairRoad(Map *map, const char *cityName1, const char *cityName2, int repairYear) {
    City *city1 = NULL;
    City *city2 = NULL;
    Road *road = NULL;

    FAIL_IF(map == NULL || repairYear == 0);
    FAIL_IF(!checkName(cityName1) || !checkName(cityName2) || strcmp(cityName1, cityName2) == 0);

    city1 = valueInDict(map->cities, cityName1);
    city2 = valueInDict(map->cities, cityName2);
    road = findRoad(city1, city2);

    FAIL_IF(city1 == NULL || city2 == NULL || road == NULL);
    FAIL_IF(road->lastRepaired > repairYear);

    road->lastRepaired = repairYear;
    return true;

    FAILURE:

    return false;
}

RoadStatus getRoadStatus(Map *map, const char *cityName1, const char *cityName2,
                         unsigned length, int repairYear) {
    City *city1 = NULL;
    City *city2 = NULL;
    Road *road = NULL;

    FAIL_IF(map == NULL || repairYear == 0 || length == 0);
    FAIL_IF(!checkName(cityName1) || !checkName(cityName2) || strcmp(cityName1, cityName2) == 0);

    city1 = valueInDict(map->cities, cityName1);
    city2 = valueInDict(map->cities, cityName2);
    road = findRoad(city1, city2);

    if (road == NULL) {
        return ROAD_ADDABLE;
    }

    FAIL_IF(road->length != length);
    if (road->lastRepaired < repairYear) {
        return ROAD_REPAIRABLE;
    }
    if (road->lastRepaired == repairYear) {
        return ROAD_EXACT;
    }

    FAILURE:

    return ROAD_ILLEGAL;
}

bool newRoute(Map *map, unsigned routeId, const char *cityName1, const char *cityName2) {
    City *city1 = NULL;
    City *city2 = NULL;
    Vector *roads = NULL;
    Route *route = NULL;

    FAIL_IF(map == NULL || !checkRouteId(routeId) || map->routes[routeId] != NULL);
    FAIL_IF(!checkName(cityName1) || !checkName(cityName2) || strcmp(cityName1, cityName2) == 0);

    city1 = valueInDict(map->cities, cityName1);
    city2 = valueInDict(map->cities, cityName2);
    FAIL_IF(city1 == NULL || city2 == NULL);

    roads = findRoute(map, city1, city2, NULL).roads;
    FAIL_IF(roads == NULL);

    route = initRoute(&roads, city1, city2);
    FAIL_IF(route == NULL);

    map->routes[routeId] = route;
    return true;

    FAILURE:

    deleteVector(roads, NULL);
    deleteRoute(route);
    return false;
}

bool createRoute(Map *map, unsigned routeId, const char **cityNames, size_t cityCount) {
    Vector *roads = NULL;
    City *firstCity = NULL;
    City *lastCity = NULL;
    size_t *usedCities = NULL;
    Route *route = NULL;

    FAIL_IF(map == NULL || !checkRouteId(routeId) || map->routes[routeId] != NULL || cityCount < 2);
    FAIL_IF(cityNames == NULL || !checkName(cityNames[0]));

    roads = initVector();
    usedCities = malloc(sizeof(size_t) * cityCount);
    FAIL_IF(roads == NULL || usedCities == NULL);

    firstCity = valueInDict(map->cities, cityNames[0]);
    /* Nie ma sprawdzenia czy miasta są NULL bo wystarczy sprawdzać drogę. */
    lastCity = firstCity;
    for (size_t i = 0; i < cityCount - 1; i++) {
        FAIL_IF(!checkName(cityNames[i + 1]));
        City *nextCity = valueInDict(map->cities, cityNames[i + 1]);
        Road *road = findRoad(lastCity, nextCity);
        FAIL_IF(road == NULL || !pushToVector(roads, road));
        usedCities[i] = lastCity->id;
        lastCity = nextCity;
    }
    usedCities[cityCount - 1] = lastCity->id;
    FAIL_IF(!checkForDuplicateIds(usedCities, cityCount));
    free(usedCities);
    usedCities = NULL;

    /* Po wykonaniu całej pętli w lastCity jest ostatnie miasto na drodze. */
    route = initRoute(&roads, firstCity, lastCity);
    FAIL_IF(route == NULL);

    map->routes[routeId] = route;
    return true;

    FAILURE:

    deleteRoute(route);
    free(usedCities);
    deleteVector(roads, NULL);
    return false;
}

bool extendRoute(Map *map, unsigned routeId, const char *cityName) {
    Vector *roads1 = NULL;
    Vector *roads2 = NULL;
    FAIL_IF(map == NULL || !checkRouteId(routeId) || !checkName(cityName));

    Route *route = map->routes[routeId];
    City *city = valueInDict(map->cities, cityName);
    FAIL_IF(city == NULL || route == NULL);

    {
        size_t usedRoadsCount = sizeOfVector(route->roads);
        Road **usedRoadsArray = (Road **) storageBlockOfVector(route->roads);
        for (size_t i = 0; i < usedRoadsCount; i++) {
            FAIL_IF(usedRoadsArray[i]->end1 == city || usedRoadsArray[i]->end2 == city);
        }
    }

    RouteSearchAnswer answer1 = findRoute(map, city, route->end1, route->roads);
    RouteSearchAnswer answer2 = findRoute(map, route->end2, city, route->roads);
    roads1 = answer1.roads;
    roads2 = answer2.roads;

    /* W którymś wyszukiwaniu nastąpił błąd. */
    FAIL_IF(answer1.count == -1 || answer2.count == -1);

    /* Żadne wyszukiwanie nie znalazło dokładnie jednego wyniku. */
    FAIL_IF(answer1.count != 1 && answer2.count != 1);

    /* Sprawdzamy do którego końca chcemy przedłużyć. */
    bool connectToEnd1;
    if (answer1.count == 1) {
        if (answer2.count == 0) {
            connectToEnd1 = true;
        } else if (answer2.count == 1) {
            int comparison = compareDistances(answer1.distance, answer2.distance);

            /* Ta sama długość, więc nie da sie wybrać. */
            FAIL_IF(comparison == 0);
            if (comparison < 0) {
                connectToEnd1 = true;
            } else {
                connectToEnd1 = false;
            }
        } else {
            /* Więcej niż jeden wynik w drugim szukaniu,
             * więc da się tylko gdy w pierwszym jest ściśle lepszy dystans. */
            int comparison = compareDistances(answer1.distance, answer2.distance);

            FAIL_IF(comparison >= 0);
            connectToEnd1 = true;
        }
    } else { /* [answer1.count != 1] czyli [answer2.count == 1]. */
        if (answer1.count == 0) {
            connectToEnd1 = false;
        } else {
            /* Więcej niż jeden wynik w pierwszym szukaniu,
             * więc da się tylko gdy w drugim jest ściśle lepszy dystans. */
            int comparison = compareDistances(answer1.distance, answer2.distance);

            FAIL_IF(comparison <= 0);
            connectToEnd1 = false;
        }
    }

    if (connectToEnd1) {
        FAIL_IF(!appendVector(roads1, route->roads));

        route->roads = roads1;
        roads1 = NULL;
        route->end1 = city;
    } else {
        FAIL_IF(!appendVector(route->roads, roads2));

        roads2 = NULL;
        route->end2 = city;
    }

    deleteVector(roads1, NULL);
    deleteVector(roads2, NULL);
    return true;

    FAILURE:

    deleteVector(roads1, NULL);
    deleteVector(roads2, NULL);
    return false;
}

bool removeRoad(Map *map, const char *cityName1, const char *cityName2) {
    Road *road = NULL;
    int oldYear = 0;
    Vector **replacementParts = NULL;
    FAIL_IF(map == NULL);
    FAIL_IF(!checkName(cityName1) || !checkName(cityName2) || strcmp(cityName1, cityName2) == 0);

    City *city1 = valueInDict(map->cities, cityName1);
    City *city2 = valueInDict(map->cities, cityName2);
    FAIL_IF(city1 == NULL || city2 == NULL);

    road = findRoad(city1, city2);
    FAIL_IF(road == NULL);

    /* Przeszukiwanie grafu nie będzie mogło użyć tego odcinka. */
    oldYear = road->lastRepaired;
    road->lastRepaired = 0;

    replacementParts = calloc(MAX_ROUTE_ID + 1, sizeof(Vector *));
    FAIL_IF(replacementParts == NULL);

    for (size_t id = 0; id <= MAX_ROUTE_ID; id++) {
        Route *route = map->routes[id];
        if (route == NULL || !existsInVector(route->roads, road)) {
            continue;
        }

        int orientation = checkRouteOrientation(route, city1, city2);

        if (orientation == 1) {
            replacementParts[id] = findRoute(map, city1, city2, route->roads).roads;
        } else {
            FAIL_IF(orientation != 2);
            replacementParts[id] = findRoute(map, city2, city1, route->roads).roads;
        }

        FAIL_IF(replacementParts[id] == NULL);
        FAIL_IF(!prepareForReplacingValueWithVector(route->roads, road, replacementParts[id]));
    }


    for (size_t id = 0; id <= MAX_ROUTE_ID; id++) {
        if (map->routes[id] != NULL && replacementParts[id] != NULL) {
            /* Jest pewność, że się powiedzie. */
            replaceValueWithVector(map->routes[id]->roads, road, replacementParts[id]);
        }
    }

    popFromVector(city1->roads, road, NULL);
    popFromVector(city2->roads, road, NULL);
    free(replacementParts);
    deleteRoad(road);
    return true;

    FAILURE:

    if (replacementParts != NULL) {
        for (size_t i = 0; i <= MAX_ROUTE_ID; i++) {
            deleteVector(replacementParts[i], NULL);
        }
    }
    free(replacementParts);
    if (road != NULL && oldYear != 0) {
        road->lastRepaired = oldYear;
    }
    return false;
}

char const *getRouteDescription(Map *map, unsigned routeId) {
    char *description = NULL;
    if (map == NULL || !checkRouteId(routeId) || map->routes[routeId] == NULL) {
        return calloc(1, sizeof(char));
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

    description = malloc(sizeof(char) * totalLength);
    FAIL_IF(description == NULL);

    description[0] = '\0';
    position = route->end1;
    char *descriptionPosition = description;
    addUnsignedToDescription(&descriptionPosition, routeId);
    for (size_t i = 0; i < roadCount; i++) {
        addNameToDescription(&descriptionPosition, position->name);
        addUnsignedToDescription(&descriptionPosition, roads[i]->length);
        addIntToDescription(&descriptionPosition, roads[i]->lastRepaired);
        position = otherRoadEnd(roads[i], position);
    }
    strcat(descriptionPosition, position->name);

    return description;

    FAILURE:

    free(description);
    return NULL;
}

bool removeRoute(Map *map, unsigned routeId) {
    if (map == NULL || !checkRouteId(routeId) || map->routes[routeId] == NULL) {
        return false;
    }

    deleteRoute(map->routes[routeId]);
    map->routes[routeId] = NULL;
    return true;
}