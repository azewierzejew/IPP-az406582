#ifndef DROGI_DICT_H
#define DROGI_DICT_H


#include "vector.h"

#include <stdbool.h>

struct DictStruct;

typedef struct DictStruct *Dict;

Dict initDict();

void deleteDict(Dict dict, void valueDestructor(void *));

bool addToDict(Dict dict, const char *word, void *value);

void *valueInDict(Dict dict, const char *word);

Vector vectorFromDict(Dict dict);


#endif //DROGI_DICT_H
