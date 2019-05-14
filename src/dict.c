#define _GNU_SOURCE

#include "dict.h"
#include "vector.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>


/* Definicje typów. */

/** Struktura odpowiadająca za jedno słowo w słowniku. */
typedef struct EntryStruct Entry;
/** Struktura odpowiadająca za kubełek słów o wspólnym haszu. */
typedef struct BucketStruct Bucket;


/* Deklaracje struktur. */

/** Przechowuje słowo ze słownika. */
struct EntryStruct {
    /** Wskaźnik na napis reprezentujący słowo. */
    char *word;
    /** Wartość przypisana danemu słowu. */
    void *value;
};

/**
 * Kubełek słów o jednakowym haszu.
 * Kubełki są układane w strukturę drzewa BST ze względu na hash.
 */
struct BucketStruct {
    /** Hasz danego kubełka. */
    uint64_t hash;
    /** Lewy kubełek-syn. */
    Bucket *left;
    /** Prawy kubełek-syn. */
    Bucket *right;
    /** Wektor słow w kubełku (typu @ref EntryStruct *). */
    Vector *entries;
};

/** Przechowuje słownik. */
struct DictStruct {
    /** Korzeń drzewa BST kubełków. */
    Bucket *root;
};


/* Stałe. */

static const uint64_t HASH_MODULO = 1770134209;
static const uint64_t HASH_START = HASH_MODULO & 0xaaaaaaaa;
static const uint64_t HASH_MULTIPLIER = 257;
static const uint64_t HASH_XOR_MASK = 0xff;


/* Funkcje pomocnicze. */

static Entry *initEntry(const char *word, void *value);

static Bucket *initBucket(uint64_t hash);

static uint64_t hashWord(const char *word);

static void deleteBucketTree(Bucket *bucket, void valueDestructor(void *));


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
        uint64_t byte = (unsigned char) word[len] ^xor;
        hash = (hash * HASH_MULTIPLIER + byte) % HASH_MODULO;
        len++;
    }

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
        valueDestructor(entries[i]->value);
    }
    deleteVector(bucket->entries, free);

    free(bucket);
}

static Bucket **findBucket(Bucket **bucketPtr, uint64_t hash) {
    if (*bucketPtr == NULL) {
        return bucketPtr;
    }

    if ((*bucketPtr)->hash < hash) {
        return findBucket(&(*bucketPtr)->right, hash);
    }

    if ((*bucketPtr)->hash > hash) {
        return findBucket(&(*bucketPtr)->left, hash);
    }

    return bucketPtr;
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
    Bucket **bucketPtr = findBucket(&dict->root, hash);
    if (*bucketPtr == NULL) {
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

void *valueInDict(Dict *dict, const char *word) {
    if (dict == NULL || word == NULL) {
        return NULL;
    }

    uint64_t hash = hashWord(word);
    Bucket *bucket = *findBucket(&dict->root, hash);
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
