/* MODULE 1A (C) - Arrays and Searching : implementation */
#include <stdio.h>
#include "organism_array.h"

void arr_init(OrganismArray *a)
{
    a->count = 0;
}

/* Add one record at the end of the array. */
int arr_add(OrganismArray *a, int id, const char *name, int type, int population)
{
    OrganismRec *r;
    if (a->count >= MAX_ORGANISMS) return 0;     
    r = &a->items[a->count];
    r->id = id;
    eco_copy_name(r->name, name);
    r->type = type;
    r->population = population;
    a->count++;
    return 1;
}

void arr_display(const OrganismArray *a)
{
    int i;
    printf("%-4s %-14s %-10s %10s\n", "ID", "Name", "Type", "Population");
    for (i = 0; i < a->count; i++) {
        const OrganismRec *r = &a->items[i];
        printf("%-4d %-14s %-10s %10d\n", r->id, r->name, eco_type_name(r->type), r->population);
    }
}

/* LINEAR SEARCH by id: check every element one by one. */
int arr_search_by_id(const OrganismArray *a, int id)
{
    int i;
    for (i = 0; i < a->count; i++)
        if (a->items[i].id == id) return i;
    return -1;
}

/* LINEAR SEARCH by name (ignores upper/lower case).*/
int arr_search_by_name(const OrganismArray *a, const char *name)
{
    int i;
    for (i = 0; i < a->count; i++)
        if (eco_stricmp(a->items[i].name, name) == 0) return i;
    return -1;
}

int arr_count_by_type(const OrganismArray *a, int type)
{
    int i, n = 0;
    for (i = 0; i < a->count; i++)
        if (a->items[i].type == type) n++;
    return n;
}

/* One pass over the array collects every statistic. O(n) */
void arr_stats(const OrganismArray *a, PopStats *out)
{
    int i;
    memset(out, 0, sizeof(PopStats));
    out->totalSpecies = a->count;
    for (i = 0; i < a->count; i++) {
        int t = a->items[i].type;
        out->totalPopulation += a->items[i].population;
        out->speciesByType[t]++;
        out->populationByType[t] += a->items[i].population;
    }
}

void arr_display_stats(const PopStats *s)
{
    int t;
    printf("Species: %d   Total population: %d\n", s->totalSpecies, s->totalPopulation);
    for (t = 0; t < TYPE_COUNT; t++)
        printf("  %-10s species=%d  individuals=%d\n", eco_type_name(t),
               s->speciesByType[t], s->populationByType[t]);
}
