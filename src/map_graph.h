/** @file
 * @author Antoni Żewierżejew <azewierzejew@gmail.com>
 * @date 05.06.2019
 */

#ifndef DROGI_MAP_GRAPH_H
#define DROGI_MAP_GRAPH_H

#include "map_types.h"

Road *initRoad(int builtYear, unsigned length, City *end1, City *end2);

void deleteRoad(void *roadVoid);

void deleteRoadHalfway(void *roadVoid);

City *initCity(const char *name, size_t id);

void deleteCity(void *cityVoid);

City *otherRoadEnd(const Road *road, const City *end);

Road *findRoad(const City *city1, const City *city2);

#endif /* DROGI_MAP_GRAPH_H */
