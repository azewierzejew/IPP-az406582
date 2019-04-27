#include "dict.h"
#include "vector.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


// Definicje typów.

typedef struct EntryStruct *Entry;


// Deklaracje struktur.

struct EntryStruct {
    char *word;
    void *value;
};

struct DictStruct {
    Vector entries;
};


// Funkcje pomocnicze.

static Entry initEntry(const char *word, void *value);

static void doNothing(__attribute__((unused)) void *arg);


// Implementacja funkcji pomocniczych.

static Entry initEntry(const char *word, void *value) {
    Entry entry = malloc(sizeof(struct EntryStruct));
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

static void doNothing(__attribute__((unused)) void *arg) {

}


// Funkcje z interfejsu.

Dict initDict() {
    Dict dict = malloc(sizeof(struct DictStruct));
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

void deleteDict(Dict dict, void valueDestructor(void *)) {
    if (dict == NULL) {
        return;
    }

    size_t entryCount = sizeOfVector(dict->entries);
    Entry *entries = (Entry *) storageBlockOfVector(dict->entries);
    for (size_t i = 0; i < entryCount; i++) {
        free(entries[i]->word);
        valueDestructor(entries[i]->value);
    }
    deleteVector(dict->entries, free);

    free(dict);
}

bool addToDict(Dict dict, const char *word, void *value) {
    if (dict == NULL) {
        return false;
    }

    Entry entry = initEntry(word, value);
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

void *valueInDict(Dict dict, const char *word) {
    if (dict == NULL) {
        return NULL;
    }

    size_t entryCount = sizeOfVector(dict->entries);
    Entry *entries = (Entry *) storageBlockOfVector(dict->entries);
    for (size_t i = 0; i < entryCount; i++) {
        if (strcmp(word, entries[i]->word) == 0) {
            return entries[i]->value;
        }
    }

    return NULL;
}

Vector vectorFromDict(Dict dict) {
    if (dict == NULL) {
        return NULL;
    }

    Vector values = initVector();
    if (values == NULL) {
        return NULL;
    }

    bool success = true;
    size_t entryCount = sizeOfVector(dict->entries);
    Entry *entries = (Entry *) storageBlockOfVector(dict->entries);
    for (size_t i = 0; i < entryCount && success; i++) {
        success = pushToVector(values, entries[i]->value);
    }

    if (!success) {
        deleteVector(values, doNothing);
        return NULL;
    }

    return values;
}