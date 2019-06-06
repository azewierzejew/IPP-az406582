/** @file
 * Implementacja klasy przechowującej słownik.
 *
 * @author Antoni Żewierżejew <azewierzejew@gmail.com>
 * @date 27.04.2019
 */

#define _GNU_SOURCE

#include "dict.h"
#include "vector.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>


/* Definicje typów. */

/** Struktura odpowiadająca za jeden wpis w słowniku. */
typedef struct EntryStruct Entry;
/** Struktura odpowiadająca za kubełek wpisów o wspólnym haszu. */
typedef struct BucketStruct Bucket;


/* Deklaracje struktur. */

/** Przechowuje wpis ze słownika. */
struct EntryStruct {
    /** Wskaźnik na napis reprezentujący słowo. */
    char *word;
    /** Wartość przypisana danemu słowu. */
    void *value;
};

/**
 * Kubełek wpisów o jednakowym haszu.
 * Kubełki są układane w strukturę drzewa BST ze względu na hasz.
 */
struct BucketStruct {
    /** Hasz danego kubełka. */
    uint64_t hash;
    /** Lewy kubełek-syn. */
    Bucket *left;
    /** Prawy kubełek-syn. */
    Bucket *right;
    /** Wektor słów w kubełku (typu @ref EntryStruct *). */
    Vector *entries;
};

/** Przechowuje słownik. */
struct DictStruct {
    /** Korzeń drzewa BST kubełków. */
    Bucket *root;
};


/* Stałe. */

/** Modulo używane przy haszowaniu. */
static const uint64_t HASH_MODULO = 1770134209;
/** Początkowa wartość hasza dla pustego słowa. */
static const uint64_t HASH_START = HASH_MODULO & 0xaaaaaaaa;
/** Mnożnik do przemnażania kolejnych liter przy haszowaniu. */
static const uint64_t HASH_MULTIPLIER = 257;
/** Maska do ekstrakcji wartości z hasza, aby sxorować z bajtem. */
static const uint64_t HASH_XOR_MASK = 0xff;


/* Funkcje pomocnicze. */

/**
 * @brief Tworzy pojedyncze wejście do słownika.
 * Tworzy wejście, czyli @ref Entry, które zawiera słowo i przypisaną mu wartość.
 * Słowo dla danego wejścia jest kopiowane do nowego miejsca w pamięci.
 * @param[in] word  - słowo;
 * @param[in] value - wartość dla słowa.
 * @return Wskaźnik na wpis lub @p NULL gdy się nie powiodło utworzenie.
 */
static Entry *initEntry(const char *word, void *value);

/**
 * @brief Tworzy kubełek wpisów.
 * Tworzy nowy, pusty kubełek wpisów i przypisuje mu dany hasz.
 * @param[in] hash - hasz kubełka.
 * @return Wskaźnik na kubełek lub @p NULL gdy się nie powiodło utworzenie.
 */
static Bucket *initBucket(uint64_t hash);

/**
 * @brief Liczy hash słowa.
 * Liczy hasz, czyli funkcję skrótu, dwa takie same słowa zawsze mają tę samą wartość.
 * @param[in] word - słowo.
 * @return hash danego słowa.
 */
static uint64_t hashWord(const char *word);

/**
 * @brief Usuwa kubełek haszy wraz z zawartością.
 * Jeśli @p valueDestructor to @p NULL nie wywołuje go.
 * @param[in,out] bucket      - wskaźnik na kubełek do usunięcia;
 * @param[in] valueDestructor - funkcja do usuwania wartości z pamięci.
 */
static void deleteBucketTree(Bucket *bucket, void valueDestructor(void *));

/**
 * @brief Znajduje kubełek o podanym hashu.
 * Mając dany hasz i wskaźnik na korzeń drzewa BST kubełków znajduje odpowiedni kubełek.
 * Jeśli taki nie istnieje to w kubełku, którego synem byłby kubełek o takim hashu
 * znajduje odpowiedni wskaźnik na syna.
 * Zwraca wskaźnik na odpowiedni wskaźnik, zatem jeśli kubełek nie istnieje to
 * zwraca wskaźnik na NULL w miejscu gdzie należałoby taki kubełek dodać.
 * Pozwala to właśnie na dodanie kubełka bez znania kontekstu w jakim się znajduje.
 * @param[in] bucketPtr - wskaźnik na wskaźnik na kubełek będący korzeniem;
 * @param[in] hash      - hasz kubełka.
 * @return Wskaźnik na wskaźnik pod którym powinien być kubełek.
 */
static Bucket **findBucketPtr(Bucket **bucketPtr, uint64_t hash);

/**
 * @brief Znajduje kubełek o podanym haszu.
 * Znajduje kubełek o podanym haszu w drzewie BST kubełków.
 * @param[in] bucket - kubełek będący korzeniem drzewa;
 * @param[in] hash   - szukany hasz.
 * @return Wskaźnik na kubełek lub @p NULL jeśli nie ma.
 */
static Bucket *findBucket(Bucket *bucket, uint64_t hash);


/* Implementacja funkcji pomocniczych. */

static Entry *initEntry(const char *word, void *value) {
    Entry *entry = malloc(sizeof(Entry));
    if (entry == NULL) {
        return NULL;
    }

    entry->word = strdup(word);
    if (entry->word == NULL) {
        free(entry);
        return NULL;
    }

    entry->value = value;
    return entry;
}

static Bucket *initBucket(uint64_t hash) {
    Bucket *bucket = malloc(sizeof(Bucket));
    if (bucket == NULL) {
        return NULL;
    }

    bucket->entries = initVector();
    if (bucket->entries == NULL) {
        free(bucket);
        return NULL;
    }

    bucket->left = NULL;
    bucket->right = NULL;
    bucket->hash = hash;
    return bucket;
}

static uint64_t hashWord(const char *word) {
    if (word == NULL) {
        return 0;
    }

    uint64_t hash = HASH_START;
    size_t len = 0;
    while (word[len] != '\0') {
        unsigned char xor = hash & HASH_XOR_MASK;
        /* Xor hasza z kodowanym bajtem, żeby hasze podobnych słów nie były w dokładnie tej samej kolejności. */
        uint64_t byte = (unsigned char) word[len] ^xor;
        hash = (hash * HASH_MULTIPLIER + byte) % HASH_MODULO;
        len++;
    }

    /* Uwzględniamy długość słowa w haszu. */
    hash = (hash * HASH_MULTIPLIER + len) % HASH_MODULO;
    return hash;
}

static void deleteBucketTree(Bucket *bucket, void valueDestructor(void *)) {
    if (bucket == NULL) {
        return;
    }

    deleteBucketTree(bucket->left, valueDestructor);
    deleteBucketTree(bucket->right, valueDestructor);

    size_t entryCount = sizeOfVector(bucket->entries);
    Entry **entries = (Entry **) storageBlockOfVector(bucket->entries);
    for (size_t i = 0; i < entryCount; i++) {
        free(entries[i]->word);
        if (valueDestructor != NULL) {
            valueDestructor(entries[i]->value);
        }
    }
    deleteVector(bucket->entries, free);

    free(bucket);
}

static Bucket **findBucketPtr(Bucket **bucketPtr, uint64_t hash) {
    if (*bucketPtr == NULL) {
        return bucketPtr;
    }

    if ((*bucketPtr)->hash < hash) {
        return findBucketPtr(&(*bucketPtr)->right, hash);
    }

    if ((*bucketPtr)->hash > hash) {
        return findBucketPtr(&(*bucketPtr)->left, hash);
    }

    return bucketPtr;
}

static Bucket *findBucket(Bucket *bucket, const uint64_t hash) {
    if (bucket == NULL) {
        return NULL;
    }

    if (bucket->hash < hash) {
        return findBucket(bucket->right, hash);
    }

    if (bucket->hash > hash) {
        return findBucket(bucket->left, hash);
    }

    return bucket;
}


/* Funkcje z interfejsu. */

Dict *initDict() {
    Dict *dict = malloc(sizeof(Dict));
    if (dict == NULL) {
        return NULL;
    }

    dict->root = NULL;
    return dict;
}

void deleteDict(Dict *dict, void valueDestructor(void *)) {
    if (dict == NULL) {
        return;
    }

    deleteBucketTree(dict->root, valueDestructor);
    free(dict);
}

bool addToDict(Dict *dict, const char *word, void *value) {
    if (dict == NULL || word == NULL || value == NULL) {
        return false;
    }

    uint64_t hash = hashWord(word);
    Bucket **bucketPtr = findBucketPtr(&dict->root, hash);
    if (*bucketPtr == NULL) {
        /* Jeśli nie ma odpowiedniego kubełka to należy go stworzyć. */
        *bucketPtr = initBucket(hash);
        if (*bucketPtr == NULL) {
            return false;
        }
    }

    Bucket *bucket = *bucketPtr;
    size_t entryCount = sizeOfVector(bucket->entries);
    Entry **entries = (Entry **) storageBlockOfVector(bucket->entries);
    for (size_t i = 0; i < entryCount; i++) {
        if (strcmp(word, entries[i]->word) == 0) {
            entries[i]->value = value;
            return true;
        }
    }

    Entry *entry = initEntry(word, value);
    if (entry == NULL) {
        return false;
    }

    if (!pushToVector(bucket->entries, entry)) {
        free(entry->word);
        free(entry);
        return false;
    }

    return true;
}

void *valueInDict(const Dict *dict, const char *word) {
    if (dict == NULL || word == NULL) {
        return NULL;
    }

    uint64_t hash = hashWord(word);
    Bucket *bucket = findBucket(dict->root, hash);
    if (bucket == NULL) {
        return NULL;
    }

    size_t entryCount = sizeOfVector(bucket->entries);
    Entry **entries = (Entry **) storageBlockOfVector(bucket->entries);
    for (size_t i = 0; i < entryCount; i++) {
        if (strcmp(word, entries[i]->word) == 0) {
            return entries[i]->value;
        }
    }

    return NULL;
}
