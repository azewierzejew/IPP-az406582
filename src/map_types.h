/** @file
 * Ogólne definicje związane z działaniem modułu map.
 *
 * @author Antoni Żewierżejew <azewierzejew@gmail.com>
 * @date 29.03.2019
 */

#ifndef DROGI_MAP_TYPES_H
#define DROGI_MAP_TYPES_H


#include "vector.h"
#include "dict.h"

#include <inttypes.h>


/* Definicje typów. */

/** Struktura przechowująca mapę dróg krajowych. */
typedef struct Map Map;

/** Struktura przechowująca informacje o odcinku drogowym. */
typedef struct RoadStruct Road;

/** Struktura przechowująca informacje o mieście. */
typedef struct CityStruct City;

/** Struktura przechowująca informacje o drodze krajowej. */
typedef struct RouteStruct Route;


/* Deklaracje struktur. */

/** Przechowuje elementy mapy. */
struct Map {
    /** Słownik, gdzie nazwie miasta jest przypisany wskaźnik na obiekt miasta. */
    Dict *cities;
    /** Wskaźnik na tablicę wskaźników na drogi krajowe. */
    Route **routes;
    /** Liczba miast na mapie. */
    size_t cityCount;
};

/** Przechowuje informacje o drodze. */
struct RoadStruct {
    /** Rok ostatniego remontu lub budowy. Jeśli jest @p 0 to droga jest w "zablokowanym" stanie. */
    int lastRepaired;
    /** Pierwszy koniec drogi. */
    City *end1;
    /** Drugi koniec drogi. */
    City *end2;
    /** Długość drogi. Jeśli jest @p 0 to droga jest niedostępna. */
    unsigned length;
};

/** Przechowuje informacje o mieście. */
struct CityStruct {
    /** Wskaźnik na napis reprezentujący nazwę miasta. */
    char *name;
    /** Wskaźnik na wektor dróg wychodzących z miasta. */
    Vector *roads;
    /** Id miasta w danej mapie. */
    size_t id;
};

/** Przechowuje informacje o drodze krajowej. */
struct RouteStruct {
    /** Początek drogi. */
    City *end1;
    /** Koniec drogi. */
    City *end2;
    /** Wektor kolejnych odcinków drogowych. */
    Vector *roads;
};

#endif /*DROGI_MAP_TYPES_H*/
