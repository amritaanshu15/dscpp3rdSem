/*
 * sort.h  -  insertion sort for the "danger radar"
 * DS topic: Sorting (insertion sort)
 *
 * Every turn we make a small array of predators with their distance
 * from the rabbit and sort it, so the nearest danger is shown first.
 */
#ifndef SORT_H
#define SORT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int  id;
    char name[20];
    int  dist;        /* steps from the rabbit (sort key) */
} RadarItem;

void insertion_sort_radar(RadarItem arr[], int n);   /* smallest distance first */

#ifdef __cplusplus
}
#endif

#endif
