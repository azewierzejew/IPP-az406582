/** @file
 * Interfejs modułu znajdującego ścieżki w grafie miast.
 *
 * @author Antoni Żewierżejew <azewierzejew@gmail.com>
 * @date 29.03.2019
 */

#ifndef DROGI_MAP_FIND_ROUTE_H
#define DROGI_MAP_FIND_ROUTE_H

#include "map_types.h"

/** Struktura przechowująca łączny dystans dla drogi. */
typedef struct DistanceStruct Distance;

/** Struktura przechowująca wynik szukania drogi. */
typedef struct RouteSearchAnswerStruct RouteSearchAnswer;

/** Przechowuje istotne dla długości drogi wartości i pozawala na nich operować. */
struct DistanceStruct {
    /** Łączna długość wszystkich odcinków. */
    uint64_t length;
    /** Data najstarszej naprawy. */
    int lastRepaired;
};

/** Zawiera istotne parametry dla wyniku szukania drogi. */
struct RouteSearchAnswerStruct {
    /** Liczba znalezionych dróg. */
    int count;
    /** Vector odcinków należących do znalezionej drogi. */
    Vector *roads;
    /** Sumaryczny dystans odcinków. */
    Distance distance;
};


/**
 * @brief Porównuje dystanse.
 * Porównuje dystanse najpierw pod względem długości a potem pod względem daty ostatniej naprawy.
 * @param distance1 - pierwszy dystans;
 * @param distance2 - drugi dystans.
 * @return @p -1 jeśli pierwszy dystans jest lepszy, @p 1 w odwrotnym wypadku,
 * @p 0 gdy są takie same.
 */
int compareDistances(Distance distance1, Distance distance2);

/**
 * @brief Szuka drogi pomiędzy dwoma miastami.
 * Dla danej mapy i miast końcowych szuka najbardziej optymalnej drogi.
 * Nie przechodzi przez miasta, które są końcem jakiejś użytej drogi.
 * @param[in] map   - wskaźnik na strukturę przechowującą mapę dróg;
 * @param[in] city1 - wskaźnik na pierwsze miasto;
 * @param[in] city2 - wskaźnik na drugie miasto;
 * @param[in] usedRoads - wskaźnik na wektor zużytych dróg (może być NULL).
 * @return struktura @ref RouteSearchAnswer z następującą wartością @ref RouteSearchAnswer.count :
 * - @p -1, jeśli nastąpił błąd lub argumenty sa niepoprawne;
 * - @p 0, jeśli nie ma żadnej drogi;
 * - @p 1, jeśli jest dokładnie jedna droga, wtedy @ref RouteSearchAnswer.roads zawiera @ref Vector odcinków,
 *   a @ref RouteSearchAnswer.distance zawiera łączny dystans;
 * - @p 2, jeśli są co najmniej dwie drogi o tym samym dystansie,
 *   wtedy @ref RouteSearchAnswer.distance zawiera ten dystans.
 */
RouteSearchAnswer findRoute(const Map *map, City *city1, City *city2, const Vector *usedRoads);

#endif /*DROGI_MAP_FIND_ROUTE_H*/
