#ifndef ORGANISM_ARRAY_H
#define ORGANISM_ARRAY_H

#include "../common/eco_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    OrganismRec items[MAX_ORGANISMS];
    int count;
} OrganismArray;

typedef struct {
    int totalSpecies;
    int totalPopulation;
    int speciesByType[TYPE_COUNT];
    int populationByType[TYPE_COUNT];
} PopStats;

void arr_init(OrganismArray *a);
int  arr_add(OrganismArray *a, int id, const char *name, int type, int population);
void arr_display(const OrganismArray *a);
int  arr_search_by_id(const OrganismArray *a, int id);
int  arr_search_by_name(const OrganismArray *a, const char *name);
int  arr_count_by_type(const OrganismArray *a, int type);
void arr_stats(const OrganismArray *a, PopStats *out);
void arr_display_stats(const PopStats *s);

#ifdef __cplusplus
}
#endif
#endif
