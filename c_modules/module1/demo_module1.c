#include <stdio.h>
#include "organism_array.h"

int main(void)
{
    OrganismArray arr;
    PopStats st;
    int idx;

    arr_init(&arr);
    arr_add(&arr, 1, "Grass",   TYPE_PLANT,     500);
    arr_add(&arr, 2, "Deer",    TYPE_HERBIVORE,  60);
    arr_add(&arr, 3, "Tiger",   TYPE_CARNIVORE,   6);
    arr_add(&arr, 4, "Sparrow", TYPE_BIRD,       80);

    arr_display(&arr);
    idx = arr_search_by_id(&arr, 3);
    printf("\nSearch id 3   -> index %d (%s)\n", idx, idx >= 0 ? arr.items[idx].name : "none");
    idx = arr_search_by_name(&arr, "deer");
    printf("Search 'deer' -> index %d\n", idx);
    printf("Herbivore species: %d\n\n", arr_count_by_type(&arr, TYPE_HERBIVORE));
    arr_stats(&arr, &st);
    arr_display_stats(&st);
    return 0;
}
