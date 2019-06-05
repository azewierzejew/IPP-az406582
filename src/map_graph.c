/** @file
 * @author Antoni Żewierżejew <azewierzejew@gmail.com>
 * @date 05.06.2019
 */

#define _GNU_SOURCE

#include "map_graph.h"
#include "map_types.h"
#include "utility.h"

#include <string.h>


/* Funkcje z interfejsu. */

Road *initRoad(int builtYear, unsigned length, City *end1, City *end2) {
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

void deleteRoad(void *roadVoid) {
    Road *road = roadVoid;
    if (road == NULL) {
        return;
    }

    free(road);
}

void deleteRoadHalfway(void *roadVoid) {
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

City *initCity(const char *name, size_t id) {
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

void deleteCity(void *cityVoid) {
    City *city = cityVoid;
    if (city == NULL) {
        return;
    }

    free(city->name);
    deleteVector(city->roads, deleteRoadHalfway);
    free(city);
}

Road *findRoad(const City *city1, const City *city2) {
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

City *otherRoadEnd(const Road *road, const City *end) {
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

