#include "dict.h"
#include "vector.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


// Definicje typów.

typedef struct EntryStruct Entry;


// Deklaracje struktur.

struct EntryStruct {
    char *word;
    void *value;
};

struct DictStruct {
    Vector *entries;
};


// Funkcje pomocnicze.

static Entry *initEntry(const char *word, void *value);


// Implementacja funkcji pomocniczych.

static Entry *initEntry(const char *word, void *value) {
    Entry *entry = malloc(sizeof(Entry));
    if (entry == NULL) {
        return NULL;
    }

    entry->word = malloc(sizeof(char) * (strlen(word) + 1));
    if (entry->word == NULL) {
        free(entry);
        return NULL;
    }
    strcpy(entry->word, word);

    entry->value = value;
    return entry;
}


// Funkcje z interfejsu.

Dict *initDict() {
    Dict *dict = malloc(sizeof(Dict));
    if (dict == NULL) {
        return NULL;
    }

    dict->entries = initVector();
    if (dict->entries == NULL) {
        free(dict);
        return NULL;
    }

    return dict;
}

void deleteDict(Dict *dict, void valueDestructor(void *)) {
    if (dict == NULL) {
        return;
    }

    size_t entryCount = sizeOfVector(dict->entries);
    Entry **entries = (Entry **) storageBlockOfVector(dict->entries);
    for (size_t i = 0; i < entryCount; i++) {
        free(entries[i]->word);
        valueDestructor(entries[i]->value);
    }
    deleteVector(dict->entries, free);

    free(dict);
}

bool addToDict(Dict *dict, const char *word, void *value) {
    if (dict == NULL || word == NULL || value == NULL) {
        return false;
    }

    if (valueInDict(dict, word) != NULL) {
        return false;
    }

    Entry *entry = initEntry(word, value);
    if (entry == NULL) {
        return false;
    }

    if (!pushToVector(dict->entries, entry)) {
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

    size_t entryCount = sizeOfVector(dict->entries);
    Entry **entries = (Entry **) storageBlockOfVector(dict->entries);
    for (size_t i = 0; i < entryCount; i++) {
        if (strcmp(word, entries[i]->word) == 0) {
            return entries[i]->value;
        }
    }

    return NULL;
}
