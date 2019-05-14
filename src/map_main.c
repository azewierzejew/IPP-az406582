#define _GNU_SOURCE

#include "map.h"
#include "vector.h"
#include "utility.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>

char *getNextParameter(char *s) {
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


static bool executeCommand(char *command, size_t len) {
    if (command == NULL || len == 0) {
        return false;
    }

    /* Usuwanie znaku newline. */
    if (command[len - 1] != '\n') {
        return false;
    }
    command[len - 1] = '\0';

    if (command[0] == '#' || len == 1) {
        return true;
    }

    Vector *params = initVector();
    if (params == NULL) {
        return false;
    }

    char *nextParameter = getNextParameter(command);
    while (nextParameter != NULL) {
        if (!pushToVector(params, nextParameter)) {
            deleteVector(params, doNothing);
        }
        nextParameter = getNextParameter(NULL);
    }
}

int main() {
    Map *map = newMap();
    if (map == NULL) {
        exit(0);
    }

    char *buff = NULL;
    size_t len = 0;
    uint64_t lineNumber = 0;

    while (getline(&buff, &len, stdin) != -1) {
        lineNumber++;
        if (!executeCommand(buff, len)) {
            fprintf(stderr, "ERROR %"PRIu64"\n", lineNumber);
        }
    }

    free(buff);
    deleteMap(map);
}