#include "find_route.h"
#include "heap.h"
#include "map_basics.h"
#include "utility.h"

#include <inttypes.h>
#include <limits.h>


/** Struktura dane potrzebne do wykorzystanie kopca w wyszukiwaniu najkrótszej drogi. */
typedef struct RouteSearchHeapEntryStruct RouteSearchHeapEntry;

/** Zawiera dane potrzebne do wykorzystania kopca, pozwalająca wybrać najkrótszą drogę z wielu. */
struct RouteSearchHeapEntryStruct {
    /** Łączny dystans drogi. */
    Distance distance;
    /** Miasto końcowe. */
    City *city;
};


/* Stałe globalne. */

static const Distance WORST_DISTANCE = {UINT64_MAX - UINT_MAX, INT_MIN};
static const Distance BASE_DISTANCE = {0, INT_MAX};


/* Funkcje pomocnicze. */

static RouteSearchHeapEntry *initHeapEntry(Distance distance, City *city);

static int compareRouteSearchHeapEntries(void *heapEntry1Void, void *heapEntry2Void);

static Distance addRoadToDistance(Distance distance, Road *road);

static Distance combineDistances(Distance distance1, Distance distance2);


/* Implementacja funkcji pomocniczych. */

static RouteSearchHeapEntry *initHeapEntry(Distance distance, City *city) {
    RouteSearchHeapEntry *entry = malloc(sizeof(RouteSearchHeapEntry));
    if (entry == NULL) {
        return NULL;
    }

    entry->distance = distance;
    entry->city = city;
    return entry;
}

static int compareRouteSearchHeapEntries(void *heapEntry1Void, void *heapEntry2Void) {
    RouteSearchHeapEntry *entry1 = heapEntry1Void;
    RouteSearchHeapEntry *entry2 = heapEntry2Void;
    if (entry1 == NULL || entry2 == NULL) {
        return 0;
    }

    return compareDistances(entry1->distance, entry2->distance);
}

static Distance addRoadToDistance(Distance distance, Road *road) {
    Distance newDistance = distance;
    newDistance.length += road->length;
    if (road->lastRepaired < newDistance.lastRepaired) {
        newDistance.lastRepaired = road->lastRepaired;
    }
    return newDistance;
}

static Distance combineDistances(Distance distance1, Distance distance2) {
    Distance newDistance = distance1;
    newDistance.length += distance2.length;
    if (distance2.lastRepaired < newDistance.lastRepaired) {
        newDistance.lastRepaired = distance2.lastRepaired;
    }
    return newDistance;
}


int compareDistances(Distance distance1, Distance distance2) {
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

RouteSearchAnswer findRoute(const Map *map, City *city1, City *city2, const Vector *usedRoads) {
    Distance *distances = NULL;
    bool *blockedCities = NULL;
    Heap *heap = NULL;
    Vector *route = NULL;

    RouteSearchAnswer answer;
    answer.count = -1;
    answer.roads = NULL;
    answer.distance = WORST_DISTANCE;
    FAIL_IF(map == NULL || city1 == NULL || city2 == NULL);

    size_t cityCount = map->cityCount;
    distances = malloc(sizeof(Distance) * cityCount);
    FAIL_IF(distances == NULL);

    for (size_t i = 0; i < cityCount; i++) {
        distances[i] = WORST_DISTANCE;
    }

    blockedCities = calloc(cityCount, sizeof(bool));
    FAIL_IF(blockedCities == NULL);
    {
        size_t usedRoadsCount = sizeOfVector(usedRoads);
        Road **usedRoadsArray = (Road **) storageBlockOfVector(usedRoads);
        for (size_t i = 0; i < usedRoadsCount; i++) {
            blockedCities[usedRoadsArray[i]->end1->id] = true;
            blockedCities[usedRoadsArray[i]->end2->id] = true;
        }
        /* Miasta końcowe mogą wystąpić na liście, więc trzeba je odznaczyć. */
        blockedCities[city1->id] = false;
        blockedCities[city2->id] = false;
    }

    heap = initHeap(compareRouteSearchHeapEntries);
    FAIL_IF(heap == NULL);

    distances[city2->id] = BASE_DISTANCE;
    {
        /* Szukana jest droga z city2 do city1, żeby odbudowując ją od tyłu była w dobrej kolejności. */
        RouteSearchHeapEntry *baseEntry = initHeapEntry(distances[city2->id], city2);
        if (baseEntry == NULL || !addToHeap(heap, baseEntry)) {
            free(distances);
            free(blockedCities);
            deleteHeap(heap, free);
            return answer;
        }
    }

    while (!isEmptyHeap(heap)) {
        RouteSearchHeapEntry *nextEntry = getMinimumFromHeap(heap);
        FAIL_IF(nextEntry == NULL);
        Distance distance = nextEntry->distance;
        City *city = nextEntry->city;
        free(nextEntry);

        if (blockedCities[city->id] || compareDistances(distance, distances[city->id]) > 0) {
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
            /* [road->length == 0] oznacza że odcinek nie jest dostępny. */
            if (road->length == 0) {
                continue;
            }

            City *newCity = otherRoadEnd(road, city);
            Distance newDistance = addRoadToDistance(distance, road);

            if (compareDistances(newDistance, distances[newCity->id]) < 0) {
                distances[newCity->id] = newDistance;
                RouteSearchHeapEntry *newEntry = initHeapEntry(newDistance, newCity);

                FAIL_IF(newEntry == NULL || !addToHeap(heap, newEntry));
            }
        }
    }

    deleteHeap(heap, free);
    heap = NULL;

    /* Żeby nie przejść przez zablokowane miasta, ustawiany jest dla nich najgorszy wynik. */
    for (size_t i = 0;
         i < cityCount; i++) {
        if (blockedCities[i]) {
            distances[i] = WORST_DISTANCE;
        }
    }

    free(blockedCities);
    blockedCities = NULL;

    route = initVector();
    FAIL_IF(route == NULL);

    City *position = city1;
    Distance currentDistance = BASE_DISTANCE;
    Distance endDistance = distances[city1->id];
    answer.distance = endDistance;
    while (position != city2) {
        City *newPosition = NULL;
        Distance newCurrentDistance = WORST_DISTANCE;
        size_t roadCount = sizeOfVector(position->roads);
        Road **roads = (Road **) storageBlockOfVector(position->roads);

        for (size_t i = 0; i < roadCount; i++) {
            Road *road = roads[i];
            if (road->length == 0) {
                continue;
            }

            City *newCity = otherRoadEnd(road, position);
            if (newCity == NULL) {
                continue;
            }
            Distance newDistance = addRoadToDistance(distances[newCity->id], road);
            newDistance = combineDistances(newDistance, currentDistance);

            /* Sprawdzenie czy da się uzyskać dobry dystans przychodząc z [newCity]. */
            if (compareDistances(newDistance, endDistance) == 0) {

                /* Jeśli [newPosition != NULL] to są dwie możliwe drogi. */
                if (newPosition != NULL || !pushToVector(route, road)) {
                    answer.count = 2;
                    FAIL;
                }
                newPosition = newCity;
                newCurrentDistance = addRoadToDistance(currentDistance, road);
            }
        }

        if (newPosition == NULL) {
            answer.count = 0;
            FAIL;
        }

        position = newPosition;
        currentDistance = newCurrentDistance;
    }

    free(distances);
    answer.roads = route;
    answer.count = 1;
    return answer;

    FAILURE:

    free(distances);
    free(blockedCities);
    deleteHeap(heap, free);
    deleteVector(route, NULL);
    return answer;
}
