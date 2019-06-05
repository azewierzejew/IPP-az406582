/** @file
 * @author Antoni Żewierżejew <azewierzejew@gmail.com>
 * @date 05.06.2019
 */

#include "map_route.h"
#include "map_types.h"
#include "map_graph.h"


/* Funkcje z interfejsu. */

Route *initRoute(Vector **const roadsPtr, City *end1, City *end2) {
    if (roadsPtr == NULL) {
        return NULL;
    }

    Route *route = malloc(sizeof(Route));
    if (route == NULL) {
        return NULL;
    }

    route->roads = *roadsPtr;
    route->end1 = end1;
    route->end2 = end2;
    *roadsPtr = NULL;
    return route;
}

void deleteRoute(void *routeVoid) {
    Route *route = routeVoid;
    if (route == NULL) {
        return;
    }

    deleteVector(route->roads, NULL);
    free(route);
}

bool checkRouteOrientation(const Route *route, const City *city1, const City *city2) {
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