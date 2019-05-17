#define _GNU_SOURCE

#include "map.h"
#include "vector.h"
#include "utility.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

static Map *map = NULL;

static char *getNextParameter(char *string);

static bool stringToUnsigned(const char *str, unsigned *number);

static bool stringToInt(const char *str, int *number);

static bool executeCommand(char *command, size_t len);

static bool createCustomRoute(unsigned routeId, const char **parameters, size_t parameterCount);

static char *getNextParameter(char *string) {
    static char *savePointer = NULL;
    if (string != NULL) {
        if (*string == '\0') {
            string = NULL;
        }
    } else {
        string = savePointer;
    }

    if (string == NULL) {
        return NULL;
    }

    char *end = string + strcspn(string, ";");
    if (*end == '\0') {
        savePointer = NULL;
        return string;
    }

    *end = '\0';
    savePointer = end + 1;
    return string;
}

static bool stringToUnsigned(const char *str, unsigned *number) {
    char *endPtr;
    errno = 0;

    /* Sprawdzamy czy nie zaczyna się białym znakiem bo strtoul tego nie testuje. */
    if (isspace(str[0])) {
        return false;
    }
    long unsigned result = strtoul(str, &endPtr, 0);
    /* Nic nie skonwertowane lub zostało coś po liczbie. */
    if (endPtr == str || *endPtr != '\0') {
        return false;
    }

    if (result > UINT_MAX || errno == ERANGE) {
        return false;
    }

    *number = result;
    return true;
}

static bool stringToInt(const char *str, int *number) {
    char *endPtr;
    errno = 0;

    /* Sprawdzamy czy nie zaczyna się białym znakiem bo strtol tego nie testuje. */
    if (isspace(str[0])) {
        return false;
    }
    long result = strtol(str, &endPtr, 0);
    /* Nic nie skonwertowane lub zostało coś po liczbie. */
    if (endPtr == str || *endPtr != '\0') {
        return false;
    }

    if (result > INT_MAX || result < INT_MIN || errno == ERANGE) {
        return false;
    }

    *number = result;
    return true;
}

static bool executeCommand(char *command, size_t len) {
    Vector *parametersVector = NULL;
    FAIL_IF(command == NULL || len == 0);

    /* Wczytano zerowy bajt, który jest niepoprawny. */
    FAIL_IF(len != strlen(command));

    /* Usuwanie znaku newline. */
    FAIL_IF(command[len - 1] != '\n');
    command[len - 1] = '\0';

    if (command[0] == '#') {
        return true;
    }

    parametersVector = initVector();
    FAIL_IF(parametersVector == NULL);

    char *nextParameter = getNextParameter(command);
    while (nextParameter != NULL) {
        FAIL_IF(!pushToVector(parametersVector, nextParameter));
        nextParameter = getNextParameter(NULL);
    }

    size_t parameterCount = sizeOfVector(parametersVector);
    const char **parameters = (const char **) storageBlockOfVector(parametersVector);
    if (parameterCount == 0) {
        return true;
    }

    /* Z komendy zostały "wyjęte" wszystkie parametry. */
    if (strcmp(command, "addRoad") == 0) {
        FAIL_IF(parameterCount != 5);
        const char *city1Name = parameters[1];
        const char *city2Name = parameters[2];
        unsigned length;
        int builtYear;
        FAIL_IF(!stringToUnsigned(parameters[3], &length));
        FAIL_IF(!stringToInt(parameters[4], &builtYear));

        deleteVector(parametersVector, NULL);
        return addRoad(map, city1Name, city2Name, length, builtYear);
    }
    if (strcmp(command, "repairRoad") == 0) {
        FAIL_IF(parameterCount != 4);
        const char *city1Name = parameters[1];
        const char *city2Name = parameters[2];
        int repairYear;
        FAIL_IF(!stringToInt(parameters[3], &repairYear));

        deleteVector(parametersVector, NULL);
        return repairRoad(map, city1Name, city2Name, repairYear);
    }
    if (strcmp(command, "getRouteDescription") == 0) {
        FAIL_IF(parameterCount != 2);
        unsigned routeId;
        FAIL_IF(!stringToUnsigned(parameters[1], &routeId));

        deleteVector(parametersVector, NULL);
        const char *description = getRouteDescription(map, routeId);
        if (description == NULL) {
            return false;
        }

        printf("%s\n", description);
        return true;
    }

    unsigned routeId;
    FAIL_IF(!stringToUnsigned(parameters[0], &routeId));
    return createCustomRoute(routeId, parameters + 1, parameterCount - 1);

    FAILURE:

    deleteVector(parametersVector, NULL);
    return false;
}

static bool createCustomRoute(unsigned routeId, const char **parameters, size_t parameterCount) {
    const char **cityNames = NULL;
    int *roadLengths = NULL;
    unsigned *roadYears = NULL;
    size_t roadCount = parameterCount / 3;
    FAIL_IF(roadCount < 1 || roadCount * 3 + 1 != parameterCount);

    cityNames = malloc(sizeof(const char *) * (roadCount + 1));
    roadLengths = malloc(sizeof(int) * roadCount);
    roadYears = malloc(sizeof(unsigned) * roadCount);
    FAIL_IF(cityNames == NULL || roadLengths == NULL || roadYears == NULL);

    for (size_t i = 0; i < roadCount; i++) {
        size_t nr = i * 3;
        cityNames[i] = parameters[nr++];
        FAIL_IF(!stringToInt(parameters[nr++], &roadLengths[i]));
        FAIL_IF(!stringToUnsigned(parameters[nr++], &roadYears[i]));
    }
    cityNames[roadCount] = parameters[roadCount * 3];

    for (size_t i = 0; i < roadCount; i++) {
        FAIL_IF(!setupRoad(map, cityNames[i], cityNames[i + 1], roadLengths[i], roadYears[i]));
    }

    FAIL_IF(!createRoute(map, routeId, cityNames, roadCount + 1));

    free(roadYears);
    free(roadLengths);
    free(cityNames);
    return true;

    FAILURE:

    free(roadYears);
    free(roadLengths);
    free(cityNames);
    return false;
}

int main() {
    map = newMap();
    if (map == NULL) {
        exit(0);
    }

    char *buff = NULL;
    size_t len = 0;
    uint64_t lineNumber = 0;
    ssize_t readLength = getline(&buff, &len, stdin);

    while (readLength >= 0) {
        lineNumber++;
        if (!executeCommand(buff, readLength)) {
            fprintf(stderr, "ERROR %"PRIu64"\n", lineNumber);
        }
        readLength = getline(&buff, &len, stdin);
    }

    free(buff);
    deleteMap(map);
}