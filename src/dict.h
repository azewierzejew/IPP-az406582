#ifndef DROGI_DICT_H
#define DROGI_DICT_H

#include <stdbool.h>

struct DictStruct;

typedef struct DictStruct *Dict;

Dict initDict();

void deleteDict(Dict dict, void valueDestructor(void *));

bool addToDict(Dict dict, char *word, void *value);

bool existsInDict(Dict dict, char *word);

void *wordValue(Dict dict, char *word);


#endif //DROGI_DICT_H
