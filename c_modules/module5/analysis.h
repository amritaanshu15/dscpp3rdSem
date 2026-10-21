#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "../common/eco_common.h"

#ifdef __cplusplus
extern "C" {
#endif

void sort_by_population(OrganismRec arr[], int n);
void sort_by_name(OrganismRec arr[], int n);

int  binary_search_by_name(const OrganismRec arr[], int n, const char *name, int *steps);

#define HASH_SIZE 11

typedef struct HashNode {
    OrganismRec rec;
    struct HashNode *next;
} HashNode;

typedef struct {
    HashNode *buckets[HASH_SIZE];
    int count;
} HashTable;

void      ht_init(HashTable *t);
int       ht_hash(const char *key);
void      ht_insert(HashTable *t, const OrganismRec *rec);
HashNode *ht_search(const HashTable *t, const char *name, int *bucket, int *steps);
void      ht_free(HashTable *t);

typedef struct {
    int    total;
    double average;
    double median;
    int    maxIndex;
    int    minIndex;
} AnalysisStats;

void analysis_stats(const OrganismRec arr[], int n, AnalysisStats *out);

#ifdef __cplusplus
}
#endif
#endif
