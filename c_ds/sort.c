/*
 * sort.c  -  insertion sort
 * Best case O(n) (already sorted), worst case O(n^2).
 * Good choice here because n is very small (max ~6 predators).
 */
#include "sort.h"

void insertion_sort_radar(RadarItem arr[], int n)
{
    int i, j;
    RadarItem key;

    for (i = 1; i < n; i++) {
        key = arr[i];
        j = i - 1;
        /* shift bigger items one place right */
        while (j >= 0 && arr[j].dist > key.dist) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}
