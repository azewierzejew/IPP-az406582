#define _GNU_SOURCE

#include "map.h"
#include "vector.h"
#include "dict.h"
#include "map_basics.h"
#include "find_route.h"
#include "utility.h"

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

static void deleteRoadHalfway(void *roadVoid);

static void deleteCity(void *cityVoid);

static void deleteRoute(void *routeVoid);

static bool correctChar(char a);

static bool checkName(const char *name);

static bool checkRouteId(unsigned routeId);

static Road *findRoad(const City *city1, const City *city2);

static City *addCity(Map *map, const char *cityName);

static void addNameToDescription(char **description, const char *name);

static void addUnsignedToDescription(char **description, unsigned number);

static void addIntToDescription(char **description, int number);

static bool checkRouteOrientation(const Route *route, const City *city1, const City *city2);


/* Implementacja funkcji pomocniczych. */

static City *initCity(const char *name, size_t id) {
    City *city = malloc(sizeof(City));
    FAIL_IF(city == NULL);

    city->name = strdup(name);
    city->roads = initVector();
    city->id = id;

    FAIL_IF(city->name == NULL || city->roads == NULL);

    return city;

    FAILURE:

    if (city != NULL) {
        free(city->name);
    }
    free(city);
    return NULL;
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

    free(road);
}

static void deleteRoadHalfway(void *roadVoid) {
    Road *road = roadVoid;
    if (road == NULL) {
        return;
    }

    /* Droga musi być usunięta z obu końców i nie ma roku 0,
     * więc rok naprawy 0 oznacza połowiczne usunięcie. */
    if (road->lastRepaired == 0) {
        deleteRoad(road);
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
    deleteVector(city->roads, deleteRoadHalfway);
    free(city);
}

static void deleteRoute(void *routeVoid) {
    Route *route = routeVoid;
    if (route == NULL) {
        return;
    }

    deleteVector(route->roads, NULL);
    free(route);
}

static bool correctChar(char a) {
    return !(0 <= a && a <= 31) && a != ';';
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

static bool checkRouteId(unsigned routeId) {
    return routeId > 0 && routeId <= MAX_ROUTE_ID;
}

static Road *findRoad(const City *city1, const City *city2) {
    if (city1 == NULL || city2 == NULL) {
        return NULL;
    }

    size_t len = sizeOfVector(city1->roads);

    /* Dla wydajności zamieniane są miasta jeśli z drugiego wychodzi mniej odcinków. */
    {
        size_t len2 = sizeOfVector(city2->roads);
        if (len > len2) {
            const City *tmp = city1;
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

static bool checkRouteOrientation(const Route *route, const City *city1, const City *city2) {
    if (route == NULL || city1 == city2) {
        return false;
    }

    const City *cityPair[2] = {city1, city2};
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
    if (position == cityPair[1]) {
        return true;
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
    deleteVector(map->doneRoutes, free);
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

bool newRoute(Map *map, unsigned routeId, const char *cityName1, const char *cityName2) {
    City *city1 = NULL;
    City *city2 = NULL;
    Vector *roads = NULL;
    Route *route = NULL;
    unsigned *id = NULL;

    FAIL_IF(map == NULL || !checkRouteId(routeId) || map->routes[routeId] != NULL);
    FAIL_IF(!checkName(cityName1) || !checkName(cityName2) || strcmp(cityName1, cityName2) == 0);

    city1 = valueInDict(map->cities, cityName1);
    city2 = valueInDict(map->cities, cityName2);
    FAIL_IF(city1 == NULL || city2 == NULL);

    roads = findRoute(map, city1, city2, NULL).roads;
    FAIL_IF(roads == NULL);

    route = initRoute(roads, city1, city2);
    FAIL_IF(route == NULL);
    roads = NULL;

    id = malloc(sizeof(unsigned));
    FAIL_IF(id == NULL);

    *id = routeId;
    FAIL_IF(!pushToVector(map->doneRoutes, id));

    map->routes[routeId] = route;
    return true;

    FAILURE:

    deleteVector(roads, NULL);
    deleteRoute(route);
    if (map != NULL) {
        popFromVector(map->doneRoutes, id, NULL);
    }
    free(id);
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
    unsigned oldLength = 0;
    Vector **replacementParts = NULL;
    FAIL_IF(map == NULL);
    FAIL_IF(!checkName(cityName1) || !checkName(cityName2) || strcmp(cityName1, cityName2) == 0);

    City *city1 = valueInDict(map->cities, cityName1);
    City *city2 = valueInDict(map->cities, cityName2);
    FAIL_IF(city1 == NULL || city2 == NULL);

    road = findRoad(city1, city2);
    FAIL_IF(road == NULL);

    /* Przeszukiwanie grafu nie będzie mogło użyć tego odcinka. */
    oldLength = road->length;
    road->length = 0;

    replacementParts = calloc(MAX_ROUTE_ID + 1, sizeof(Vector *));
    FAIL_IF(replacementParts == NULL);

    size_t doneRouteCount = sizeOfVector(map->doneRoutes);
    unsigned **doneRoutes = (unsigned **) storageBlockOfVector(map->doneRoutes);
    for (size_t i = 0; i < doneRouteCount; i++) {
        unsigned id = *doneRoutes[i];
        Route *route = map->routes[id];
        if (route == NULL || !existsInVector(route->roads, road)) {
            continue;
        }

        if (checkRouteOrientation(route, city1, city2)) {
            replacementParts[id] = findRoute(map, city1, city2, route->roads).roads;
        } else {
            replacementParts[id] = findRoute(map, city2, city1, route->roads).roads;
        }

        FAIL_IF(replacementParts[id] == NULL);
        FAIL_IF(!prepareForReplacingValueWithVector(route->roads, road, replacementParts[id]));
    }


    for (size_t i = 0; i < doneRouteCount; i++) {
        unsigned id = *doneRoutes[i];
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
    if (road != NULL && oldLength != 0) {
        road->length = oldLength;
    }
    return false;
}

char const *getRouteDescription(const Map *map, unsigned routeId) {
    char *description = NULL;
    FAIL_IF(map == NULL || !checkRouteId(routeId));

    if (map->routes[routeId] == NULL) {
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