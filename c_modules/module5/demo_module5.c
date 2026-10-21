#include <stdio.h>
#include "analysis.h"

static void show(const OrganismRec a[], int n)
{
    int i;
    for (i = 0; i < n; i++) printf("  %-8s %d\n", a[i].name, a[i].population);
}

int main(void)
{
    OrganismRec a[4] = {
        {1, "Grass", TYPE_PLANT, 500}, {2, "Deer", TYPE_HERBIVORE, 60},
        {3, "Tiger", TYPE_CARNIVORE, 6}, {4, "Sparrow", TYPE_BIRD, 80}
    };
    HashTable ht;
    HashNode *found;
    AnalysisStats st;
    int i, bucket, steps, idx;

    sort_by_population(a, 4);
    printf("Sorted by population (bubble sort):\n"); show(a, 4);

    sort_by_name(a, 4);
    printf("Sorted by name (selection sort):\n"); show(a, 4);

    idx = binary_search_by_name(a, 4, "Sparrow", &steps);
    printf("Binary search 'Sparrow' -> index %d in %d steps\n", idx, steps);

    ht_init(&ht);
    for (i = 0; i < 4; i++) ht_insert(&ht, &a[i]);
    found = ht_search(&ht, "tiger", &bucket, &steps);
    printf("Hash search 'tiger' -> %s (bucket %d, %d comparisons)\n",
           found ? "found" : "missing", bucket, steps);

    analysis_stats(a, 4, &st);
    printf("Total=%d  Average=%.1f  Median=%.1f  Max=%s  Min=%s\n",
           st.total, st.average, st.median, a[st.maxIndex].name, a[st.minIndex].name);
    ht_free(&ht);
    return 0;
}
