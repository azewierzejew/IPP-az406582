#ifndef DROGI_LIST_H
#define DROGI_LIST_H

#include <stdbool.h>

struct ListStruct;

typedef struct ListStruct *List;

List initList();

void deleteList(List list, void valueDestructor(void *));

bool addToList(List list, void *value);

bool existsInList(List list, void *value);

bool replace(List list, void *value, List *part);

#endif //DROGI_LIST_H
