/** @file
 * Interfejs klasy przechowującej minimalny kopiec.
 *
 * @author Antoni Żewierżejew <azewierzejew@gmail.com>
 * @date 29.03.2019
 */

#ifndef DROGI_HEAP_H
#define DROGI_HEAP_H

#include <stdbool.h>

/** Struktura przechowująca kopiec. */
typedef struct HeapStruct Heap;

/**
 * @brief Tworzy nowy kopiec.
 * Tworzy pusty kopiec przypisując mu komparator definiujący porządek.
 * Komparator powinien zwracać ujemną liczbę gdy pierwszy element jest mniejszy,
 * dodatnią gdy jest większy i @p 0 gdy są równe.
 * @param[in] comparator - komparator do porównywania wartości.
 * @return Wskaźnik na kopiec, @p NULL w wypadku niepowodzenia.
 */
Heap *initHeap(int comparator(void *, void *));

/**
 * @brief Usuwa kopiec.
 * Usuwa kopiec,wywołując @p valueDestructor dla każdej wartości na kopcu,
 * chyba że jest @p NULL, wtedy nie wywołuje.
 * @param[in,out] heap        - wskaźnik na kopiec;
 * @param[in] valueDestructor - funkcja do usuwania wartości.
 */
void deleteHeap(Heap *heap, void valueDestructor(void *));

/**
 * Sprawdza czy kopiec jest pusty.
 * @param[in] heap - wskaźnik na kopiec.
 * @return @p true lub @p false w zależności od stanu kopca.
 */
bool isEmptyHeap(const Heap *heap);

/**
 * Dodaje nienullową wartość do kopca i zeruje wskaźnik pod podanym adresem.
 * @param[in,out] heap     - wskaźnik na kopiec;
 * @param[in,out] valuePtr - wskaźnik na wartość.
 * @return @p true lub @p false w zależności od tego czy dodawanie się powiodło.
 */
bool addToHeap(Heap *heap, void **valuePtr);

/**
 * @brief Zwraca najmniejszą wartość na kopcu.
 * Zwraca najmniejszą wartość i usuwa ją z kopca.
 * W wypadku kilku takich samych zwraca i usuwa jedną z nich.
 * @param[in,out] heap - wskaźnik na kopiec.
 * @return najmniejsze wartość na kopcu lub @p NULL kiedy się nie da.
 */
void *getMinimumFromHeap(Heap *heap);

#endif /* DROGI_HEAP_H */
