/** @file
 * Interfejs modułu odpowiedzialnego za operacje na drogach krajowych.
 *
 * @author Antoni Żewierżejew <azewierzejew@gmail.com>
 * @date 05.06.2019
 */

#ifndef DROGI_MAP_ROUTE_H
#define DROGI_MAP_ROUTE_H

#include "map_types.h"

#include <stdbool.h>

/**
 * @brief Tworzy nową drogę krajową.
 * Tworzy nową drogę krajową o podanych końcach i danych drogach.
 * Nie wykonuje sprawdzenia poprawności tych parametrów.
 * Przyjmuje wskaźnik na oryginalny wskaźnik na wektor
 * i w wypadku powodzenia nadpisuje oryginalny wskaźnik na @p NULL;
 * @param[in,out] roadsPtr - wskaźnik na miejsce zapisu wskaźnika na wektor;
 * @param[in] end1         - początek drogi;
 * @param[in] end2         - koniec drogi.
 * @return Wskaźnik na nową drogę krajową lub @p NULL.
 */
Route *initRoute(Vector **roadsPtr, City *end1, City *end2);

/**
 * @brief Usuwa drogę krajową.
 * Usuwa daną drogę krajową i wektor odcinków, ale nic nie robi z samymi odcinkami.
 * Przyjmuje (void *) dla zgodności z generycznymi modułami.
 * @param[in,out] routeVoid - wskaźnik na drogę.
 */
void deleteRoute(void *routeVoid);

/**
 * @brief Sprawdza orientację drogi krajowej.
 * Zaczyna iść od początku drogi odpowiednimi odcinkami i szuka podanych miast.
 * Pozwala na to, żeby jakieś odcinki na drodze były zablokowane, wtedy na nich przerywa.
 * @param[in] route - droga do sprawdzenia;
 * @param[in] city1 - pierwsze szukane miasto;
 * @param[in] city2 - drugie szukane miasto.
 * @return Numer pierwszego z podanych miast na jakie natrafi (czyli 1 lub 2).
 * Jeśli nie trafi na żadne lub oba są takie same zwraca 0.
 */
int checkRouteOrientation(const Route *route, const City *city1, const City *city2);

/**
 * @brief Udostępnia informacje o drodze krajowej.
 * Zwraca wskaźnik na napis, który zawiera informacje o drodze krajowej. Alokuje
 * pamięć na ten napis. Zaalokowaną pamięć trzeba zwolnić za pomocą funkcji free.
 * Informacje wypisywane są w formacie:
 * numer drogi krajowej;nazwa miasta;długość odcinka drogi;rok budowy lub
 * ostatniego remontu;nazwa miasta;długość odcinka drogi;rok budowy lub
 * ostatniego remontu;nazwa miasta;…;nazwa miasta.
 * Kolejność miast na liście jest taka, aby miasta @p city1 i @p city2, podane
 * w wywołaniu funkcji @ref newRoute, które utworzyło tę drogę krajową, zostały
 * wypisane w tej kolejności.
 * Zakłada, że droga jest kompletna i nic nie jest zablokowane.
 * @param[in] route   - wskaźnik na drogę krajową;
 * @param[in] routeId - numer drogi krajowej.
 * @return Wskaźnik na napis lub @p NULL, gdy nie udało się zaalokować pamięci.
 */
char *generateRouteDescription(const Route *route, unsigned routeId);

#endif /* DROGI_MAP_ROUTE_H */
