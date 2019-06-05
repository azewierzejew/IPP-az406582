/** @file
 * Program pozwalający wykonywać na pewne komendy na jednej mapie dróg.
 *
 * @author Antoni Żewierżejew <azewierzejew@gmail.com>
 * @date 18.05.2019
 */

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


/* Zmienne globalne. */

/**
 * Wskaźnik na strukturę mapy, na której wykonywane są wszystkie operacje.
 */
static Map *map = NULL;


/* Funkcje pomocnicze. */

/**
 * @brief Ekstrahuje kolejne parametry z komendy.
 * Dla napisu będącego komendą wyciąga kolejne napisy pomiędzy średnikami.
 * Jeśli parametr @p string jest @p NULL to z napisu, na którym ostatnio było operowane, zwraca kolejny parametr.
 * Jeśli nie to zaczyna od nowa parsowanie podanego napisu.
 * Zamienia średniki na zerowe bajty i zwraca wskaźniki na kolejne pozycje w oryginalnym napisie.
 * @param[in,out] string - napis to ekstrahowania parametrów lub @p NULL jeśli ma być użyty poprzedni napis.
 * @return wskaźnik na odpowiednią pozycję w oryginalnym napisie lub @p NULL jeśli się skończył.
 */
static char *getNextParameter(char *string);

/**
 * @brief Konwertuje napis na @p unsigned.
 * Parsuje dany napis i jego wartość zapisuje pod podanym wskaźnikiem.
 * Napis powinien być kodowany w bazie 8, 10 lub 16.
 * Dowolne nadmiarowe znaki są uznawane za błąd.
 * @param[in] str     - napis do skonwertowania;
 * @param[out] number - wskaźnik do zapisania wartości
 * @return @p true lub @p false w zależności od powodzenia.
 */
static bool stringToUnsigned(const char *str, unsigned *number);

/**
 * @brief Konwertuje napis na @p int.
 * Parsuje dany napis i jego wartość zapisuje pod podanym wskaźnikiem.
 * Napis powinien być kodowany w bazie 8, 10 lub 16.
 * Dowolne nadmiarowe znaki są uznawane za błąd.
 * @param[in] str     - napis do skonwertowania;
 * @param[out] number - wskaźnik do zapisania wartości
 * @return @p true lub @p false w zależności od powodzenia.
 */
static bool stringToInt(const char *str, int *number);

/**
 * @brief Wykonuje komendę na mapie dróg.
 * Dla danego napisu zawierającego linię z komendą i jej długości wykonuje odpowiednią komendę.
 * Nie usuwa napisu, ale może go modyfikować.
 * @param[in,out] command - napis zawierający linię;
 * @param[in] len         - długość linii.
 * @return @p true lub @p false w zależności od powodzenia.
 */
static bool executeCommand(char *command, size_t len);

/**
 * @brief Wykonuje komendę skonstruowania konkretnej drogi.
 * Dla danego ID drogi, tablicy parametrów komendy i ilości parametrów
 * wykonuje operację stworzenia drogi.
 * Tworzy na mapie drogę krajową o podanym opisie.
 * Tworzy lub naprawia odpowiednie odcinki drogowe.
 * Może je modyfikować nawet w przypadku nieudanego stworzenia drogi krajowej.
 * @param[in] routeId        - ID dodawanej drogi;
 * @param[in] parameters     - tablica parametrów;
 * @param[in] parameterCount - liczba parametrów.
 * @return @p true lub @p false w zależności od powodzenia.
 */
static bool executeCreateRoute(unsigned routeId, const char **parameters, size_t parameterCount);


/* Implementacja funkcji pomocniczych. */

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

    /* Sprawdzamy czy nie zaczyna się białym znakiem bo strtoul tego nie testuje. */
    if (isspace(str[0])) {
        return false;
    }
    errno = 0;
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

    /* Sprawdzamy czy nie zaczyna się białym znakiem bo strtol tego nie testuje. */
    if (isspace(str[0])) {
        return false;
    }
    errno = 0;
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
        deleteVector(parametersVector, NULL);
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
        parametersVector = NULL;
        char *description = (char *) getRouteDescription(map, routeId);
        FAIL_IF(description == NULL);

        printf("%s\n", description);
        free(description);
        return true;
    }
    if (strcmp(command, "newRoute") == 0) {
        FAIL_IF(parameterCount != 4);
        unsigned routeId;
        FAIL_IF(!stringToUnsigned(parameters[1], &routeId));
        const char *city1Name = parameters[2];
        const char *city2Name = parameters[3];

        deleteVector(parametersVector, NULL);
        return newRoute(map, routeId, city1Name, city2Name);
    }
    if (strcmp(command, "extendRoute") == 0) {
        FAIL_IF(parameterCount != 3);
        unsigned routeId;
        FAIL_IF(!stringToUnsigned(parameters[1], &routeId));
        const char *cityName = parameters[2];

        deleteVector(parametersVector, NULL);
        return extendRoute(map, routeId, cityName);
    }
    if (strcmp(command, "removeRoad") == 0) {
        FAIL_IF(parameterCount != 3);
        const char *city1Name = parameters[1];
        const char *city2Name = parameters[2];

        deleteVector(parametersVector, NULL);
        return removeRoad(map, city1Name, city2Name);
    }
    if (strcmp(command, "removeRoute") == 0) {
        FAIL_IF(parameterCount != 2);
        unsigned routeId;
        FAIL_IF(!stringToUnsigned(parameters[1], &routeId));

        deleteVector(parametersVector, NULL);
        return removeRoute(map, routeId);
    }

    unsigned routeId;
    FAIL_IF(!stringToUnsigned(parameters[0], &routeId));
    bool result = executeCreateRoute(routeId, parameters + 1, parameterCount - 1);
    deleteVector(parametersVector, NULL);
    return result;

    FAILURE:

    deleteVector(parametersVector, NULL);
    return false;
}

static bool executeCreateRoute(unsigned routeId, const char **parameters, size_t parameterCount) {
    const char **cityNames = NULL;
    unsigned *roadLengths = NULL;
    int *roadYears = NULL;
    RoadStatus *roadStatuses = NULL;
    size_t roadCount = parameterCount / 3;
    FAIL_IF(roadCount < 1 || roadCount * 3 + 1 != parameterCount);

    cityNames = malloc(sizeof(const char *) * (roadCount + 1));
    roadLengths = malloc(sizeof(int) * roadCount);
    roadYears = malloc(sizeof(unsigned) * roadCount);
    FAIL_IF(cityNames == NULL || roadLengths == NULL || roadYears == NULL);

    for (size_t i = 0; i < roadCount; i++) {
        size_t nr = i * 3;
        cityNames[i] = parameters[nr++];
        FAIL_IF(!stringToUnsigned(parameters[nr++], &roadLengths[i]));
        FAIL_IF(!stringToInt(parameters[nr++], &roadYears[i]));
    }
    cityNames[roadCount] = parameters[roadCount * 3];

    roadStatuses = malloc(sizeof(RoadStatus) * roadCount);
    FAIL_IF(roadStatuses == NULL);
    for (size_t i = 0; i < roadCount; i++) {
        roadStatuses[i] = getRoadStatus(map, cityNames[i], cityNames[i + 1], roadLengths[i], roadYears[i]);
        FAIL_IF(roadStatuses[i] == ROAD_ILLEGAL);
    }

    for (size_t i = 0; i < roadCount; i++) {
        switch (roadStatuses[i]) {
            case ROAD_REPAIRABLE:
                FAIL_IF(!repairRoad(map, cityNames[i], cityNames[i + 1], roadYears[i]));
                break;
            case ROAD_ADDABLE:
                FAIL_IF(!addRoad(map, cityNames[i], cityNames[i + 1],
                                 roadLengths[i], roadYears[i]));
                break;
            default:
                break;
        }
    }

    FAIL_IF(!createRoute(map, routeId, cityNames, roadCount + 1));

    free(roadStatuses);
    free(roadYears);
    free(roadLengths);
    free(cityNames);
    return true;

    FAILURE:

    free(roadStatuses);
    free(roadYears);
    free(roadLengths);
    free(cityNames);
    return false;
}


/**
 * Funkcja main programu.
 * @return Kod wyjścia.
 */
int main() {
    map = newMap();
    if (map == NULL) {
        return 0;
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
    return 0;
}