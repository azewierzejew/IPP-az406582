/** @file
 * Interfejs modułu odpowiedzialnego za sprawdzanie poprawności argumentów.
 *
 * @author Antoni Żewierżejew <azewierzejew@gmail.com>
 * @date 05.06.2019
 */

#ifndef DROGI_MAP_CHECKERS_H
#define DROGI_MAP_CHECKERS_H

#include <stdlib.h>
#include <stdbool.h>

/**
 * Stała oznaczająca maksymalny dopuszczalny numer drogi krajowej.
 */
extern const unsigned MAX_ROUTE_ID;
/**
 * Maksymalna długość napisu reprezentującego rok naprawy drogi.
 */
extern const size_t MAX_ROUTE_ID_LENGTH;

/**
 * @brief Sprawdza poprawność nazwy.
 * Sprawdza czy wskaźnik nie jest @p NULL, czy nazwa nie jest pusta
 * i czy nie zawiera nielegalnych znaków.
 * @param[in] name - wskaźnik na napis z nazwą.
 * @return @p true lub @p false w zależności czy nazwa jest poprawna.
 */
bool checkName(const char *name);

/**
 * @brief Sprawdza poprawność indeksu drogi.
 * To znaczy, że indeks jest dodatni i nie większy niż @ref MAX_ROUTE_ID.
 * @param[in] routeId - indeks drogi.
 * @return @p true lub @p false w zależności czy indeks jest poprawny.
 */
bool checkRouteId(unsigned routeId);

#endif /* DROGI_MAP_CHECKERS_H */
