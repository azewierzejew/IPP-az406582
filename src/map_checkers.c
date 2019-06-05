/** @file
 * Implementacja modułu odpowiedzialnego za sprawdzanie poprawności argumentów.
 *
 * @author Antoni Żewierżejew <azewierzejew@gmail.com>
 * @date 05.06.2019
 */

#include "map_checkers.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


/* Stałe eksportowane. */

const unsigned MAX_ROUTE_ID = 999;


/* Funkcje pomocnicze. */

/**
 * @brief Dla znaku sprawdza czy jest dopuszczalnym znakiem w nazwie.
 * @param[in] a - znak
 * @return @p true lub @p false w zależności czy znak jest dopuszczalny.
 */
static bool correctChar(char a);


/* Implementacja funkcji pomocniczych. */

static bool correctChar(char a) {
    return !(0 <= a && a <= 31) && a != ';';
}


/* Funkcje z interfejsu. */

bool checkName(const char *name) {
    if (name == NULL) {
        return false;
    }

    if (name[0] == '\0') {
        return false;
    }

    for (const char *it = name; *it != '\0'; it++) {
        if (!correctChar(*it)) {
            return false;
        }
    }
    return true;
}

bool checkRouteId(unsigned routeId) {
    return routeId > 0 && routeId <= MAX_ROUTE_ID;
}