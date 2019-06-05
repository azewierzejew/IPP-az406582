/** @file
 * Interfejs modułu obsługującego graf mapy, czyli miasta i odcinki drogowe.
 *
 * @author Antoni Żewierżejew <azewierzejew@gmail.com>
 * @date 05.06.2019
 */

#ifndef DROGI_MAP_GRAPH_H
#define DROGI_MAP_GRAPH_H

#include "map_types.h"

/**
 * @brief Tworzy nową drogę.
 * @param[in] builtYear - rok budowy drogi,
 * @param[in] length    - długość drogi,
 * @param[in] end1      - jeden koniec drogi,
 * @param[in] end2      - drogi koniec drogi.
 * @return Wskaźnik na nową drogą albo @p NULL jeśli zabrakło pamięci.
 */
Road *initRoad(int builtYear, unsigned length, City *end1, City *end2);

/**
 * @brief Usuwa drogę z pamięci.
 * Przyjmuje (void *) dla zgodności z generycznymi modułami.
 * @param[in,out] roadVoid - wskaźnik na drogę do usunięcia.
 */
void deleteRoad(void *roadVoid);

/**
 * @brief Usuwa drogę w połowie.
 * Jeśli droga jest w zablokowanym stanie to ją usuwa,
 * a jeśli nie jest to ją blokuje.
 * Przyjmuje (void *) dla zgodności z generycznymi modułami.
 * @param[in,out] roadVoid - wskaźnik na drogę do usunięcia.
 */
void deleteRoadHalfway(void *roadVoid);

/**
 * @brief Tworzy nowe miasto.
 * Kopiuje przy tym nazwę miasta.
 * @param[in] name - nazwa miasta,
 * @param[in] id   - indeks miasta.
 * @return Wskaźnik na nowe miasto albo @p NULL jeśli zabrakło pamięci.
 */
City *initCity(const char *name, size_t id);

/**
 * @brief Usuwa miasto z pamięci.
 * Przyjmuje (void *) dla zgodności z generycznymi modułami.
 * @param[in] cityVoid - wskaźnik na miasto do usunięcia.
 */
void deleteCity(void *cityVoid);

/**
 * @brief Znajduje drugi koniec drogi.
 * Dla podanej drogi i miasta znajduje drugi koniec drogi.
 * Jeśli
 * @param[in] road - drugi koniec.
 * @param[in] end  - pierwszy koniec.
 * @return Wskaźnik na drugi koniec drogi, chyba że droga jest zablokowana,
 * nie istnieje lub @p end nie jest jej końcem zwraca @p NULL.
 */
City *otherRoadEnd(const Road *road, const City *end);

/**
 * @brief znajduje drogę pomiędzy miastami.
 * Dla danych dwóch miast znajduje drogę, która je łączy (i nie jest zablokowana).
 * Jeśli jest wiele takich dróg zwraca dowolną.
 * @param[in] city1 - jeden koniec drogi,
 * @param[in] city2 - drugi koniec drogi.
 * @return Wskaźnik na drogę lub @p NULL jeśli takiej nie ma.
 */
Road *findRoad(const City *city1, const City *city2);

#endif /* DROGI_MAP_GRAPH_H */
