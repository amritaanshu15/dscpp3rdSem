#include <stdio.h>
#include <stdlib.h>
#include "pop_list.h"

void list_init(PopList *l)
{
    l->head = NULL;
    l->count = 0;
}

int list_insert(PopList *l, int id, const char *name, int type, double population)
{
    PopNode *node, *cur;

    if (list_search(l, id) != NULL) return 0;      
    node = (PopNode *)malloc(sizeof(PopNode));      
    if (node == NULL) return 0;
    node->id = id;
    eco_copy_name(node->name, name);
    node->type = type;
    node->population = population;
    node->next = NULL;

    if (l->head == NULL) {
        l->head = node;                             
    } else {
        cur = l->head;
        while (cur->next != NULL) cur = cur->next;  
        cur->next = node;
    }
    l->count++;
    return 1;
}


int list_delete(PopList *l, int id)
{
    PopNode *cur = l->head, *prev = NULL;

    while (cur != NULL && cur->id != id) {          
        prev = cur;
        cur = cur->next;
    }
    if (cur == NULL) return 0;                     

    if (prev == NULL) l->head = cur->next;         
    else              prev->next = cur->next;       
    free(cur);                                      
    l->count--;
    return 1;
}

PopNode *list_search(const PopList *l, int id)
{
    PopNode *cur = l->head;
    while (cur != NULL) {
        if (cur->id == id) return cur;
        cur = cur->next;
    }
    return NULL;
}

int list_update_population(PopList *l, int id, double population)
{
    PopNode *n = list_search(l, id);
    if (n == NULL) return 0;
    n->population = population;
    return 1;
}

int list_count(const PopList *l)
{
    return l->count;
}

void list_display(const PopList *l)
{
    const PopNode *cur = l->head;
    while (cur != NULL) {
        printf("[%d %s pop=%.0f] -> ", cur->id, cur->name, cur->population);
        cur = cur->next;
    }
    printf("NULL\n");
}

void list_free(PopList *l)
{
    PopNode *cur = l->head, *next;
    while (cur != NULL) {
        next = cur->next;
        free(cur);
        cur = next;
    }
    l->head = NULL;
    l->count = 0;
}
