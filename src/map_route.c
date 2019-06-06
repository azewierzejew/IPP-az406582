/** @file
 * Implementacja modułu odpowiedzialnego za operacje na drogach krajowych.
 *
 * @author Antoni Żewierżejew <azewierzejew@gmail.com>
 * @date 05.06.2019
 */

#define _GNU_SOURCE

#include "map_route.h"
#include "map_types.h"
#include "map_graph.h"
#include "map_checkers.h"

#include "utility.h"

#include <string.h>
#include <stdio.h>


/* Stałe globalne. */

/** Maksymalna długość napisu reprezentującego długość drogi. */
static const size_t MAX_LENGTH_LENGTH = 10;
/** Maksymalna długość napisu reprezentującego rok naprawy drogi. */
static const size_t MAX_YEAR_LENGTH = 11;


/* Funkcje pomocnicze. */

/**
 * @brief Dodaje do opisu nazwę miasta.
 * Dodaje na podane miejsce odpowiedni napis i przesuwa wskaźnik na nowy koniec.
 * @param[in,out] description - wskaźnik na oryginalny wskaźnik na napis;
 * @param[in] name            - nazwa miasta.
 */
static void addNameToDescription(char **description, const char *name);

/**
 * @brief Dodaje do opisu liczbę bez znaku.
 * Dodaje na podane miejsce odpowiedni napis i przesuwa wskaźnik na nowy koniec.
 * @param[in,out] description - wskaźnik na oryginalny wskaźnik na napis;
 * @param[in] number          - liczba.
 */
static void addUnsignedToDescription(char **description, unsigned number);

/**
 * @brief Dodaje do opisu liczbę z znakiem.
 * Dodaje na podane miejsce odpowiedni napis i przesuwa wskaźnik na nowy koniec.
 * @param[in,out] description - wskaźnik na oryginalny wskaźnik na napis;
 * @param[in] number          - liczba.
 */
static void addIntToDescription(char **description, int number);


/* Implementacja funkcji pomocniczych. */

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

Route *initRoute(Vector **roadsPtr, City *end1, City *end2) {
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

int checkRouteOrientation(const Route *route, const City *city1, const City *city2) {
    if (route == NULL || city1 == city2) {
        return 0;
    }

    size_t roadCount = sizeOfVector(route->roads);
    Road **roads = (Road **) storageBlockOfVector(route->roads);

    City *position = route->end1;
    for (size_t i = 0; i < roadCount && position != NULL; i++) {
        if (position == city1) {
            return 1;
        }
        if (position == city2) {
            return 2;
        }
        position = otherRoadEnd(roads[i], position);
    }

    if (position == city1) {
        return 1;
    }
    if (position == city2) {
        return 2;
    }

    return 0;
}

char *generateRouteDescription(const Route *route, unsigned routeId) {
    char *description = NULL;
    if (route == NULL) {
        return calloc(1, sizeof(char));
    }

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