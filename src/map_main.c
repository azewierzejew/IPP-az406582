#define _GNU_SOURCE

#include "map.h"
#include "vector.h"
#include "utility.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>

static char *getNextParameter(char *s) {
    static char *savePointer = NULL;
    if (s != NULL) {
        if (*s == '\0') {
            s = NULL;
        }
    } else {
        s = savePointer;
    }

    if (s == NULL) {
        return NULL;
    }

    char *end = s + strcspn(s, ";");
    if (*end == '\0') {
        savePointer = NULL;
        return s;
    }

    *end = '\0';
    savePointer = end + 1;
    return s;
}

static bool stringToUnsigned(const char *str, unsigned *number) {
    char dump[2];
    if (sscanf(str, "%u%1s", number, dump) != 1) {
        return false;
    }

    return true;
}

static bool stringToInt(const char *str, int *number) {
    char dump[2];
    if (sscanf(str, "%d%1s", number, dump) != 1) {
        return false;
    }

    return true;
}

static bool executeCommand(Map *map, char *command, size_t len) {
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

    if (strcmp(parameters[0], "addRoad") == 0) {
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

    FAILURE:

    deleteVector(parametersVector, NULL);
    return false;
}

int main() {
    Map *map = newMap();
    if (map == NULL) {
        exit(0);
    }

    char *buff = NULL;
    size_t len = 0;
    uint64_t lineNumber = 0;
    ssize_t readLength = getline(&buff, &len, stdin);

    while (readLength >= 0) {
        lineNumber++;
        if (!executeCommand(map, buff, readLength)) {
            fprintf(stderr, "ERROR %"PRIu64"\n", lineNumber);
        }
        readLength = getline(&buff, &len, stdin);
    }

    free(buff);
    deleteMap(map);
}