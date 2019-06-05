/** @file
 * @author Antoni Żewierżejew <azewierzejew@gmail.com>
 * @date 05.06.2019
 */

#ifndef DROGI_MAP_ROUTE_H
#define DROGI_MAP_ROUTE_H

#include "map_types.h"

#include <stdbool.h>


Route *initRoute(Vector **const roadsPtr, City *end1, City *end2);

void deleteRoute(void *routeVoid);

bool checkRouteOrientation(const Route *route, const City *city1, const City *city2);

#endif /* DROGI_MAP_ROUTE_H */
