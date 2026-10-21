#ifndef ECO_COMMON_H
#define ECO_COMMON_H

#include <string.h>
#include <ctype.h>

#define MAX_ORGANISMS 20
#define NAME_LEN      32

#define TYPE_PLANT     0
#define TYPE_HERBIVORE 1
#define TYPE_CARNIVORE 2
#define TYPE_BIRD      3
#define TYPE_COUNT     4

typedef struct {
    int  id;
    char name[NAME_LEN];
    int  type;
    int  population;
} OrganismRec;

static inline int eco_stricmp(const char *a, const char *b)
{
    while (*a && *b) {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if (ca != cb) return ca - cb;
        a++; b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

static inline void eco_copy_name(char *dest, const char *src)
{
    strncpy(dest, src, NAME_LEN - 1);
    dest[NAME_LEN - 1] = '\0';
}

static inline const char *eco_type_name(int type)
{
    switch (type) {
        case TYPE_PLANT:     return "Plant";
        case TYPE_HERBIVORE: return "Herbivore";
        case TYPE_CARNIVORE: return "Carnivore";
        case TYPE_BIRD:      return "Bird";
    }
    return "Unknown";
}

#endif
