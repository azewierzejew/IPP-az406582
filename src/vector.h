/** @file
 * Interfejs klasy będącej wektorem, czyli tablicą dynamicznej długości.
 *
 * @author Antoni Żewierżejew <azewierzejew@gmail.com>
 * @date 29.03.2019
 */

#ifndef __DROGI_VECTOR_H__
#define __DROGI_VECTOR_H__

#include <stdlib.h>
#include <stdbool.h>

/** Struktura przechowująca wektor. */
typedef struct VectorStruct Vector;

/**
 * @brief Tworzy nowy, pusty wektor.
 * @return Wskaźnik na wektor lub @p NULL gdy brak pamięci.
 */
Vector *initVector();

/**
 * @brief Usuwa wektor.
 * Usuwa wektor, wywołując @p valueDestructor dla każdej wartości.
 * Jeśli wektor to @p NULL nic nie robi, jeśli funkcja to @p NULL nie używa jej.
 * @param[in,out] vector      - wskaźnik na wektor;
 * @param[in] valueDestructor - funkcja do usuwania wartości z pamięci.
 */
void deleteVector(Vector *vector, void valueDestructor(void *));

/**
 * @brief Dodaje wartość na koniec wektora.
 * Jeśli wektor lub wartość to @p NULL zwraca to nie dodaje.
 * @param[in,out] vector - wskaźnik na wektor;
 * @param[in] value      - wartość do dodania.
 * @return Wartość @p true jeśli się powiodło dodawanie, @p false w p.p.
 */
bool pushToVector(Vector *vector, void *value);

/**
 * @brief Usuwa wartość z wektora.
 * Usuwa z wektora daną wartość, przeglądając zawartość od tyłu.
 * Usuwa tylko pierwsze napotkane wystąpienie wartości.
 * Jeśli @p valueDestructor nie jest @p NULL wywołuje na usuwanej wartości.
 * @param[in,out] vector      - wskaźnik na wektor;
 * @param[in] value           - wartość do usunięcia;
 * @param[in] valueDestructor - funkcja do usuwania wartości.
 */
void popFromVector(Vector *vector, void *value, void valueDestructor(void *));

/**
 * Liczy rozmiar wektora.
 * @param[in] vector - wskaźnik na wektor.
 * @return Liczbę elementów na wektorze.
 */
size_t sizeOfVector(Vector *vector);

/**
 * Sprawdza czy na wektorze znajdują się elementy.
 * @param[in] vector - wskaźnik na wektor.
 * @return @p true lub @p false w zależności od tego czy są elementy.
 */
bool isEmptyVector(Vector *vector);

/**
 * Zwraca wskaźnik na blok pamięci na którym trzymane są wartości wektora.
 * Nie należy samodzielnie usuwać tego bloku pamięci.
 * @param[in] vector - wskaźnik na wektor.
 * @return Wskaźnik na blok pamięci, lub @p NULL gdy blok nie istnieje.
 */
void **storageBlockOfVector(Vector *vector);

/**
 * @brief Na miejsce wartości podstawia zawartość @p part.
 * Znajduje w wektorze pierwsze od końca wystąpienie danej wartości.
 * Usuwa tą wartość, a w jej miejsce wstawia wszystkie elementy z @p part.
 * Następnie usuwa wektor @p part.
 * @param[in,out] vector - wskaźnik na wektor;
 * @param[in] value      - wartość wskazująca miejsce;
 * @param[in,out] part   - wskaźnik na wektor z elementami do dodania.
 * @return @p true jeśli się udało, @p false jeśli dane są niepoprawne lub
 * zabrakło pamięci.
 */
bool replaceValueWithVector(Vector *vector, void *value, Vector *part);

/**
 * Przygotowuje wektor na podmiankę wartości z użyciem @ref replaceValueWithVector.
 * @param[in,out] vector - wskaźnik na wektor;
 * @param[in] value      - wartość wskazująca miejsce;
 * @param[in] part       - wskaźnik na wektor z elementami do dodania.
 * @return @p false jeśli się nie udało.
 * @p true jeśli się udało, wtedy jest gwarancja, że jeśli wektory nie zostaną zmodyfikowane,
 *   to wywołanie @ref replaceValueWithVector z tymi argumentami się powiedzie.
 */
bool prepareForReplacingValueWithVector(Vector *vector, void *value, Vector *part);

/**
 * Sprawdza czy wartość zawiera się w wektorze.
 * @param[in] vector - wskaźnik na wektor;
 * @param[in] value  - wartość do szukania.
 * @return @p true jeśli wartość się zawiera, @p false w p.p.
 */
bool existsInVector(Vector *vector, void *value);

/**
 * @brief Na koniec jednego wektora wrzuca zawartość drugiego.
 * Wszystkie elementy w @p part wrzuca do @p vector.
 * Jeśli któreś jest @p NULL lub zabraknie pamięci
 * to operacja kończy się niepowodzeniem.
 * @param[in,out] vector - wektor do którego dodawane są elementy;
 * @param[in,out] part   - wektor z którego bierzemy zawartość.
 * @return @p false lub @p true w zależności czy operacja zakończyła się powodzeniem.
 */
bool appendVector(Vector *vector, Vector *part);

#endif /*__DROGI_VECTOR_H__ */
