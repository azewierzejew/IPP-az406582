/** @file
 * Implementacja modułu znajdującego ścieżki w grafie miast.
 *
 * @author Antoni Żewierżejew <azewierzejew@gmail.com>
 * @date 29.03.2019
 */

#include "map_find_route.h"
#include "map_types.h"
#include "map_graph.h"

#include "heap.h"
#include "utility.h"

#include <inttypes.h>
#include <limits.h>


/** Struktura dane potrzebne do wykorzystanie kopca w wyszukiwaniu najkrótszej drogi. */
typedef struct RouteSearchHeapEntryStruct RouteSearchHeapEntry;

/** Zawiera dane potrzebne do wykorzystania kopca w algorytmie Dijkstry. */
struct RouteSearchHeapEntryStruct {
    /** Łączny dystans drogi. */
    Distance distance;
    /** Miasto końcowe. */
    City *city;
};


/* Stałe globalne. */

/** Stała oznaczająca najgorszy możliwy dystans. */
static const Distance WORST_DISTANCE = {UINT64_MAX - UINT_MAX, INT_MIN};
/** Stała oznaczająca dystans punktu do siebie samego. */
static const Distance BASE_DISTANCE = {0, INT_MAX};


/* Funkcje pomocnicze. */

/**
 * @brief Tworzy nowy wpis do kopca.
 * Tworzy nowy wpis do kopca używanego w algorytmie Dijkstry.
 * Te wpisy można potem wrzucać na kopiec i zdejmować wpis z najmniejszym dystansem.
 * Wpisy te można usuwać zwykłem @p free.
 * @param[in] distance - dystans dla danego wpisu;
 * @param[in] city     - miasto do którego można dojść.
 * @return Wskaźnik na nowy wpis lub @p NULL jeśli zabrakło pamięci.
 */
static RouteSearchHeapEntry *initHeapEntry(Distance distance, City *city);

/**
 * @brief Komparator wpisów (@ref RouteSearchHeapEntry) do użycia w kopcu.
 * @param[in] heapEntry1Void - pierwszy wpis;
 * @param[in] heapEntry2Void - drugi wpis.
 * @return @p -1, @p 0 lub @p 1 w zależności od stosunku wpisów.
 */
static int compareRouteSearchHeapEntries(void *heapEntry1Void, void *heapEntry2Void);

/**
 * @brief Dodaje drogę do dystansu.
 * Do odległości dodaje długość odcinka oraz bierze minimum z roku naprawy odcinka i roku naprawy w dystansie.
 * @param[in] distance - dystans;
 * @param[in] road     - odcinek drogowy.
 * @return Sumaryczny dystans.
 */
static Distance addRoadToDistance(Distance distance, Road *road);

/**
 * @brief Dodaje dwa dystanse.
 * Działa tak samo jak @ref addRoadToDistance, ale dodaje dystans a nie odcinek.
 * @param[in] distance1 - pierwszy dystans;
 * @param[in] distance2 - drugi dystans.
 * @return Sumaryczny dystans.
 */
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
    /*
     * Do szukania najkrótszej ścieżki wykorzystywany jest algorytm Dijkstry.
     * Jest to wariant kopcowy, czyli dystanse do rozpatrzenia wrzucamy na minimalny kopiec.
     * Dystanse są wrzucane dla każdej poprawy jaką da się zrobić, czyli może jedno miasto być kilka razy na kopcu.
     * Jednak wtedy dystanse będą różne (ściśle mniejsze z każdym kolejnym dodaniem).
     */
    Distance *distances = NULL;
    bool *blockedCities = NULL;
    Heap *heap = NULL;
    Vector *route = NULL;
    RouteSearchHeapEntry *entry = NULL;

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

    /* Dla danych użytych już odcinków, nie można przechodzić przez miasta na tychże odcinkach. */
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
        entry = initHeapEntry(distances[city2->id], city2);
        FAIL_IF(entry == NULL || !addToHeap(heap, (void **) &entry));
    }

    while (!isEmptyHeap(heap)) {
        RouteSearchHeapEntry *nextEntry = getMinimumFromHeap(heap);
        FAIL_IF(nextEntry == NULL);
        Distance distance = nextEntry->distance;
        City *city = nextEntry->city;
        free(nextEntry);

        if (blockedCities[city->id] || compareDistances(distance, distances[city->id]) > 0) {
            /* Nie można tędy przejść lub dystans jest nieoptymalny,
             * czyli wierzchołek już był rozważony wcześniej. */
            continue;
        }

        distances[city->id] = distance;

        if (city == city1) {
            /* Została już znaleziona cała ścieżka, więc nie ma potrzeby kontynuować. */
            break;
        }

        size_t roadCount = sizeOfVector(city->roads);
        Road **roads = (Road **) storageBlockOfVector(city->roads);
        for (size_t i = 0; i < roadCount; i++) {
            Road *road = roads[i];
            City *newCity = otherRoadEnd(road, city);
            if (newCity == NULL) {
                continue;
            }

            Distance newDistance = addRoadToDistance(distance, road);

            if (compareDistances(newDistance, distances[newCity->id]) < 0) {
                /* Da się uzyskać lepszy dystans do newCity, czyli dodawane jest wejście na kopiec. */
                distances[newCity->id] = newDistance;
                entry = initHeapEntry(newDistance, newCity);

                FAIL_IF(entry == NULL || !addToHeap(heap, (void **) &entry));
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
        /* Idąc od końca sprawdzane jest skąd mógł zostać uzyskany dystans i na ile sposobów. */
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
                    answer.count = 2;
                    FAIL;
                }
                newPosition = newCity;
                newCurrentDistance = addRoadToDistance(currentDistance, road);
            }
        }

        if (newPosition == NULL) {
            /* Nie ma żadnego rozwiązania. */
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

    free(entry);
    free(distances);
    free(blockedCities);
    deleteHeap(heap, free);
    deleteVector(route, NULL);
    return answer;
}
