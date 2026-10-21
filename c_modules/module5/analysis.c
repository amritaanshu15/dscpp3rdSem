#include <stdio.h>
#include <stdlib.h>
#include "analysis.h"


void sort_by_population(OrganismRec arr[], int n)
{
    int i, j;
    for (i = 0; i < n - 1; i++)
        for (j = 0; j < n - 1 - i; j++)
            if (arr[j].population < arr[j + 1].population) {
                OrganismRec t = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = t;
            }
}

void sort_by_name(OrganismRec arr[], int n)
{
    int i, j, smallest;
    for (i = 0; i < n - 1; i++) {
        smallest = i;
        for (j = i + 1; j < n; j++)
            if (eco_stricmp(arr[j].name, arr[smallest].name) < 0) smallest = j;
        if (smallest != i) {
            OrganismRec t = arr[i];
            arr[i] = arr[smallest];
            arr[smallest] = t;
        }
    }
}


int binary_search_by_name(const OrganismRec arr[], int n, const char *name, int *steps)
{
    int low = 0, high = n - 1;
    *steps = 0;
    while (low <= high) {
        int mid = (low + high) / 2;
        int cmp = eco_stricmp(name, arr[mid].name);
        (*steps)++;
        if (cmp == 0) return mid;
        if (cmp < 0) high = mid - 1;
        else         low = mid + 1;
    }
    return -1;
}


void ht_init(HashTable *t)
{
    int i;
    for (i = 0; i < HASH_SIZE; i++) t->buckets[i] = NULL;
    t->count = 0;
}

int ht_hash(const char *key)
{
    unsigned long h = 0;
    while (*key) {
        h = h * 31 + (unsigned long)tolower((unsigned char)*key);
        key++;
    }
    return (int)(h % HASH_SIZE);
}

void ht_insert(HashTable *t, const OrganismRec *rec)
{
    int b = ht_hash(rec->name);
    HashNode *node = (HashNode *)malloc(sizeof(HashNode));
    if (node == NULL) return;
    node->rec = *rec;
    node->next = t->buckets[b];
    t->buckets[b] = node;
    t->count++;
}

HashNode *ht_search(const HashTable *t, const char *name, int *bucket, int *steps)
{
    HashNode *cur;
    *bucket = ht_hash(name);
    *steps = 0;
    for (cur = t->buckets[*bucket]; cur != NULL; cur = cur->next) {
        (*steps)++;
        if (eco_stricmp(cur->rec.name, name) == 0) return cur;
    }
    return NULL;
}

void ht_free(HashTable *t)
{
    int i;
    for (i = 0; i < HASH_SIZE; i++) {
        HashNode *cur = t->buckets[i], *next;
        while (cur != NULL) {
            next = cur->next;
            free(cur);
            cur = next;
        }
        t->buckets[i] = NULL;
    }
    t->count = 0;
}


void analysis_stats(const OrganismRec arr[], int n, AnalysisStats *out)
{
    OrganismRec sorted[MAX_ORGANISMS];
    int i;

    out->total = 0; out->average = 0; out->median = 0;
    out->maxIndex = -1; out->minIndex = -1;
    if (n <= 0) return;

    out->maxIndex = 0; out->minIndex = 0;
    for (i = 0; i < n; i++) {
        out->total += arr[i].population;
        if (arr[i].population > arr[out->maxIndex].population) out->maxIndex = i;
        if (arr[i].population < arr[out->minIndex].population) out->minIndex = i;
        sorted[i] = arr[i];
    }
    out->average = (double)out->total / n;

    sort_by_population(sorted, n);                
    if (n % 2 == 1) out->median = sorted[n / 2].population;
    else            out->median = (sorted[n / 2 - 1].population + sorted[n / 2].population) / 2.0;
}
