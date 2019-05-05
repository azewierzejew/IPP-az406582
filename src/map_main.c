#define _GNU_SOURCE

#include "map.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>

static size_t findSemicolon(const char *string) {
    size_t w = 0;
    while (string[w] != ';' && string[w] != '\0') w++;
    return w;
}

static bool executeCommand(const char *command, size_t len) {
    if (command == NULL || len == 0) {
        return false;
    }

    if (command[len - 1] != '\n') {
        return false;
    }

    if (command[0] == '#' || len == 1) {
        return true;
    }

    size_t firstSemicolon = findSemicolon(command);
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