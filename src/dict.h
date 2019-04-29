/** @file
 * Interfejs klasy przechowującej słownik.
 *
 * @author Antoni Żewierżejew <azewierzejew@gmail.com>
 * @date 29.03.2019
 */

#ifndef __DROGI_DICT_H__
#define __DROGI_DICT_H__

#include "vector.h"

#include <stdbool.h>

/** Struktura przechowująca słowink. */
typedef struct DictStruct Dict;

/**
 * @brief Tworzy nowy słownik bez żadnych słów.
 * @return Wskaźnik na utworzony słownik lub @p NULL, gdy nie udało się
 * zaalokować pamięci.
 */
Dict *initDict();

/**
 * @brief Usuwa słownik.
 * Usuwa słownik wskazywany przez @p dict.
 * Na każdej wartości wywołuje .@p valueDestructor
 * @param[in,out] dict        - wskaźnik na słownik;
 * @param[in] valueDestructor - funkcja do usuwania wartości z pamięci.
 */
void deleteDict(Dict *dict, void valueDestructor(void *));

/**
 * @brief Dodaje do słownika słowo.
 * Dla danego słowa, przypisuje mu w słowniku niezerową wartość.
 * @param[in,out] dict - wskaźnik na słownik;
 * @param[in] word     - wskaźnik na napis reprezentujący słowo;
 * @param[in] value    - wartość do przypisania
 * @return Wartość @p true, jeśli słowo zostało dodane.
 * Wartość @p false, jeśli wystąpił błąd: argumenty są niepoprawne lub brak pamięci.
 */
bool addToDict(Dict *dict, const char *word, void *value);

/**
 * @brief Udostępnia wartość słowa w słowniku.
 * @param[in] dict - wskaźnik na słownik;
 * @param[in] word - wskaźnik na napis reprezentujący słowo.
 * @return znalezioną wartość, @p NULL jeśli nie ma słowa w słowniku.
 */
void *valueInDict(Dict *dict, const char *word);


#endif /* __DROGI_DICT_H__ */
