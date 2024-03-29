/** @file
 * Interfejs klasy przechowującej słownik.
 *
 * @author Antoni Żewierżejew <azewierzejew@gmail.com>
 * @date 29.03.2019
 */

#ifndef DROGI_DICT_H
#define DROGI_DICT_H

#include <stdbool.h>

/** Struktura przechowująca słownik. */
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
 * Na każdej wartości wywołuje @p valueDestructor, chyba że jest @p NULL.
 * @param[in,out] dict        - wskaźnik na słownik;
 * @param[in] valueDestructor - funkcja do usuwania wartości z pamięci.
 */
void deleteDict(Dict *dict, void valueDestructor(void *));

/**
 * @brief Dodaje do słownika słowo.
 * Dla danego słowa, przypisuje mu w słowniku nienullową wartość.
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
void *valueInDict(const Dict *dict, const char *word);


#endif /* DROGI_DICT_H */
