#ifndef POP_LIST_H
#define POP_LIST_H

#include "../common/eco_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PopNode {
    int    id;
    char   name[NAME_LEN];
    int    type;
    double population;
    struct PopNode *next;
} PopNode;

typedef struct {
    PopNode *head;
    int      count;
} PopList;

void     list_init(PopList *l);
int      list_insert(PopList *l, int id, const char *name, int type, double population);
int      list_delete(PopList *l, int id);
PopNode *list_search(const PopList *l, int id);
int      list_update_population(PopList *l, int id, double population);
int      list_count(const PopList *l);
void     list_display(const PopList *l);
void     list_free(PopList *l);

#ifdef __cplusplus
}
#endif
#endif
