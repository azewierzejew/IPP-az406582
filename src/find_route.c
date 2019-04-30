#include "find_route.h"
#include "heap.h"
#include "map_basics.h"

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


static const Distance WORST_DISTANCE = {UINT64_MAX - UINT_MAX, INT_MIN};
static const Distance BASE_DISTANCE = {0, INT_MAX};


static RouteSearchHeapEntry *initHeapEntry(Distance distance, City *city);

static int compareRouteSearchHeapEntries(void *heapEntry1Void, void *heapEntry2Void);

static Distance addRoadToDistance(Distance distance, Road *road);

static Distance combineDistances(Distance distance1, Distance distance2);

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

RouteSearchAnswer findRoute(Map *map, City *city1, City *city2, Vector *usedRoads) {
    RouteSearchAnswer answer;
    answer.count = -1;
    answer.roads = NULL;
    answer.distance = WORST_DISTANCE;
    if (map == NULL || city1 == NULL || city2 == NULL) {
        return answer;
    }

    size_t cityCount = map->cityCount;
    Distance *distances = malloc(sizeof(Distance) * cityCount);
    if (distances == NULL) {
        return answer;
    }

    for (size_t i = 0; i < cityCount; i++) {
        distances[i] = WORST_DISTANCE;
    }

    bool *blockedCities = calloc(cityCount, sizeof(bool));
    if (blockedCities == NULL) {
        free(distances);
        return answer;
    }
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

    Heap *heap = initHeap(compareRouteSearchHeapEntries);
    if (heap == NULL) {
        free(distances);
        free(blockedCities);
        return answer;
    }
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

                if (newEntry == NULL || !addToHeap(heap, newEntry)) {
                    free(distances);
                    free(blockedCities);
                    deleteHeap(heap, free);
                    return answer;
                }
            }
        }
    }

    deleteHeap(heap, free);

    /* Żeby nie przejść przez zablokowane miasta, ustawiany jest dla nich najgorszy wynik. */
    for (size_t i = 0;
         i < cityCount; i++) {
        if (blockedCities[i]) {
            distances[i] = WORST_DISTANCE;
        }
    }

    free(blockedCities);

    Vector *route = initVector();
    if (route == NULL) {
        free(distances);
        return answer;
    }

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
                    free(distances);
                    deleteVector(route, doNothing);
                    answer.count = 2;
                    return answer;
                }
                newPosition = newCity;
                newCurrentDistance = addRoadToDistance(currentDistance, road);
            }
        }

        if (newPosition == NULL) {
            free(distances);
            deleteVector(route, doNothing);
            answer.count = 0;
            return answer;
        }

        position = newPosition;
        currentDistance = newCurrentDistance;
    }

    free(distances);
    answer.roads = route;
    answer.count = 1;
    return answer;
}
