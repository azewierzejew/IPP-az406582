#include "map.h"
#include "vector.h"
#include "dict.h"
#include "heap.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <inttypes.h>


// Definicje typów.

typedef struct RoadStruct Road;
typedef struct CityStruct City;
typedef struct RouteStruct Route;
typedef struct DistanceStruct Distance;
typedef struct RouteSearchHeapEntryStruct RouteSearchHeapEntry;


// Deklaracje struktur.
struct RoadStruct {
    int lastRepaired;
    unsigned length;
    City *end1;
    City *end2;
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

struct DistanceStruct {
    uint64_t length;
    int lastRepaired;
};

struct RouteSearchHeapEntryStruct {
    Distance distance;
    City *city;
};


// Stałe globalne.

static const Distance WORST_DISTANCE = {UINT64_MAX, INT_MIN};
static const Distance BASE_DISTANCE = {0, INT_MAX};
static const unsigned MAX_ROUTE_ID = 999;
static const size_t MAX_LENGTH_LENGTH = 10;
static const size_t MAX_YEAR_LENGTH = 11;
static const size_t MAX_ROUTE_ID_LENGTH = 3;


// Funkcje pomocnicze.

static City *initCity(const char *name, size_t id);

static Road *initRoad(int builtYear, unsigned length, City *end1, City *end2);

static Route *initRoute(Vector *roads, City *end1, City *end2);

static RouteSearchHeapEntry *initHeapEntry(Distance distance, City *city);

static Distance addRoadToDistance(Distance distance, Road *road);

static void doNothing(void *arg);

static void deleteRoad(void *roadVoid);

static void deleteCity(void *cityVoid);

static void deleteRoute(void *routeVoid);

static bool correctChar(char a);

static bool checkName(const char *name);

static City *otherRoadEnd(Road *road, City *end);

static Road *findRoad(City *city1, City *city2);

static int compareDistances(Distance distance1, Distance distance2);

static int compareRouteSearchHeapEntries(void *heapEntry1Void, void *heapEntry2Void);

static Vector *findRoute(Map *map, City *city1, City *city2, Vector *usedRoads);

static City *addCity(Map *map, const char *cityName);

static void addNameToDescription(char **description, char *name);

static void addUnsignedToDescription(char **description, unsigned number);

static void addIntToDescription(char **description, int number);

static Distance calculateDistance(Vector *roadsVector);

// Implementacja funkcji pomocniczych.

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

static RouteSearchHeapEntry *initHeapEntry(Distance distance, City *city) {
    RouteSearchHeapEntry *entry = malloc(sizeof(RouteSearchHeapEntry));
    if (entry == NULL) {
        return NULL;
    }

    entry->distance = distance;
    entry->city = city;
    return entry;
}

static Distance addRoadToDistance(Distance distance, Road *road) {
    Distance newDistance = distance;
    newDistance.length += road->length;
    if (road->lastRepaired <
        distance.lastRepaired) {
        newDistance.lastRepaired = road->lastRepaired;
    }
    return newDistance;
}

static void doNothing(__attribute__((unused)) void *arg) {}

static void deleteRoad(void *roadVoid) {
    Road *road = roadVoid;
    if (road == NULL) {
        return;
    }

    /* Droga musi być usunięta z obu końców i nie ma roku 0,
     * więc rok naprawy 0 oznacza połowiczne usunięcie. */
    if (road->lastRepaired == 0) { free(road); } else { road->lastRepaired = 0; }
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

static City *otherRoadEnd(Road *road, City *end) {
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

static Road *findRoad(City *city1, City *city2) {
    if (city1 == NULL || city2 == NULL) {
        return NULL;
    }

    size_t len = sizeOfVector(city1->roads);

    // Dla wydajności zamieniane są miasta jeśli z drugiego wychodzi mniej odcinków.
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

static int compareDistances(Distance distance1, Distance distance2) {
    if (distance1.length < distance2.length) {
        return -1;
    }
    if (distance1.length > distance2.length) {
        return 1;
    }

    if (distance1.lastRepaired > distance2.lastRepaired) {
        return -1;
    }
    if (distance1.lastRepaired < distance2.lastRepaired) {
        return 1;
    }
    return 0;
}

static int compareRouteSearchHeapEntries(void *heapEntry1Void, void *heapEntry2Void) {
    RouteSearchHeapEntry *entry1 = heapEntry1Void;
    RouteSearchHeapEntry *entry2 = heapEntry2Void;
    if (entry1 == NULL || entry2 == NULL) {
        return 0;
    }

    return compareDistances(entry1->distance, entry2->distance);
}

static Vector *findRoute(Map *map, City *city1, City *city2, Vector *usedRoads) {
    if (map == NULL || city1 == NULL || city2 == NULL) {
        return NULL;
    }

    size_t cityCount = map->cityCount;
    Distance *distances = malloc(sizeof(Distance) * cityCount);
    if (distances == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < cityCount; i++) {
        distances[i] = WORST_DISTANCE;
    }

    bool *blockedCities = calloc(cityCount, sizeof(bool));
    if (blockedCities == NULL) {
        free(distances);
        return NULL;
    }
    {
        size_t usedRoadsCount = sizeOfVector(usedRoads);
        Road **usedRoadsArray = (Road **) storageBlockOfVector(usedRoads);
        for (size_t i = 0; i < usedRoadsCount; i++) {
            blockedCities[usedRoadsArray[i]->end1->id] = true;
            blockedCities[usedRoadsArray[i]->end2->id] = true;
        }
        // Miasta końcowe mogą wystąpić na liście, więc trzeba je odznaczyć.
        blockedCities[city1->id] = false;
        blockedCities[city2->id] = false;
    }

    Heap *heap = initHeap(compareRouteSearchHeapEntries);
    if (heap == NULL) {
        free(distances);
        free(blockedCities);
        return NULL;
    }
    distances[city2->id] = BASE_DISTANCE;
    {
        // Szukana jest droga z city2 do city1, żeby odbudowywując ją od tyłu była w dobrej kolejności.
        RouteSearchHeapEntry *baseEntry = initHeapEntry(distances[city2->id], city2);
        if (baseEntry == NULL || !addToHeap(heap, baseEntry)) {
            free(distances);
            free(blockedCities);
            deleteHeap(heap, free);
            return NULL;
        }
    }
    while (!isEmptyHeap(heap)) {
        RouteSearchHeapEntry *nextEntry = getMinimumFromHeap(heap);
        Distance distance = nextEntry->distance;
        City *city = nextEntry->city;
        free(nextEntry);

        if (blockedCities[city->id] ||
            compareDistances(distance, distances[city->id]) > 0) {
            continue;
        }

        distances[city->id] = distance;

        if (city == city1) {
            break;
        }


        size_t roadCount = sizeOfVector(city->roads);
        Road **roads = (Road **) storageBlockOfVector(city->roads);
        for (size_t i = 0; i < roadCount; i++) {
            Road *road = roads[i];
            City *newCity = otherRoadEnd(road, city);
            Distance newDistance = addRoadToDistance(distance, road);

            if (compareDistances(newDistance, distances[newCity->id]) < 0) {
                distances[newCity->id] = newDistance;
                RouteSearchHeapEntry *newEntry = initHeapEntry(newDistance, newCity);

                if (newEntry == NULL || !addToHeap(heap, newEntry)) {
                    free(distances);
                    free(blockedCities);
                    deleteHeap(heap, free);
                    return NULL;
                }
            }
        }
    }

    deleteHeap(heap, free);
    heap = NULL;

    // Żeby nie przejść przez zablokowane miasta, ustawiany jest dla nich najgorszy wynik.
    for (size_t i = 0;
         i < cityCount; i++) {
        if (blockedCities[i]) {
            distances[i] = WORST_DISTANCE;
        }
    }

    free(blockedCities);
    blockedCities = NULL;

    Vector *route = initVector();
    if (route == NULL) {
        free(distances);
        return NULL;
    }

    City *position = city1;
    while (position != city2) {
        City *newPosition = NULL;
        size_t roadCount = sizeOfVector(position->roads);
        Road **roads = (Road **) storageBlockOfVector(position->roads);

        for (size_t i = 0; i < roadCount; i++) {
            Road *road = roads[i];
            City *newCity = otherRoadEnd(road, position);
            Distance newDistance = addRoadToDistance(distances[newCity->id], road);

            // Sprawdzenie czy da się uzyskać dobry dystans przychodząc z [newCity].
            if (compareDistances(newDistance, distances[position->id]) == 0) {

                // Jeśli [newPosition != NULL] to są dwie możliwe drogi.
                if (newPosition != NULL || !pushToVector(route, road)) {
                    free(distances);
                    deleteVector(route, doNothing);
                    return NULL;
                }
                newPosition = newCity;
            }
        }

        if (newPosition == NULL) {
            free(distances);
            deleteVector(route, doNothing);
            return NULL;
        }

        position = newPosition;
    }

    free(distances);
    return route;
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

static Distance calculateDistance(Vector *roadsVector) {
    Distance distance = BASE_DISTANCE;
    if (roadsVector == NULL) {
        return distance;
    }

    size_t roadCount = sizeOfVector(roadsVector);
    Road **roads = (Road **) storageBlockOfVector(roadsVector);

    for (size_t i = 0; i < roadCount; i++) {
        distance = addRoadToDistance(distance, roads[i]);
    }

    return distance;
}


// Funkcje z interfejsu.

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
    if (map == NULL || builtYear == 0) {
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

    Vector *roads = findRoute(map, city1, city2, NULL);
    if (roads == NULL) {
        return false;
    }

    Route *route = initRoute(roads, city1, city2);
    if (route == NULL) {
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

    if (!checkName(cityName) || map->routes[routeId] != NULL) {
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

    Vector *roads1 = findRoute(map, city, route->end1, route->roads);
    Vector *roads2 = findRoute(map, city, route->end1, route->roads);

    bool connectToEnd1;
    if (roads1 == NULL) {
        if (roads2 == NULL) {
            return false;
        } else {
            connectToEnd1 = false;
        }
    } else {
        if (roads2 == NULL) {
            connectToEnd1 = true;
        } else {
            Distance distance1 = calculateDistance(roads1);
            Distance distance2 = calculateDistance(roads2);

            int comparison = compareDistances(distance1, distance2);
            if (comparison < 0) {
                connectToEnd1 = true;
            } else if (comparison > 0) {
                connectToEnd1 = false;
            } else {
                deleteVector(roads1, doNothing);
                deleteVector(roads2, doNothing);
                return false;
            }

        }
    }

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
    }

    return true;
}

bool removeRoad(Map *map, const char *cityName1, const char *cityName2) {
    // TODO napisz kod removeRoad
}

char const *getRouteDescription(Map *map, unsigned routeId) {
    if (map == NULL || routeId == 0 || routeId > MAX_ROUTE_ID || map->routes[routeId] == NULL) {
        return NULL;
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