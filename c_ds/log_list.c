/*
 * log_list.c  -  linked list of messages
 */
#include <stdlib.h>
#include <string.h>
#include "log_list.h"

void log_init(LogList *l)
{
    l->head = NULL;
    l->count = 0;
}

int log_add(LogList *l, const char *text)
{
    LogNode *node = (LogNode *)malloc(sizeof(LogNode));
    if (node == NULL)
        return 0;                              /* no memory = overflow */

    strncpy(node->text, text, MSG_LEN - 1);
    node->text[MSG_LEN - 1] = '\0';

    /* insert at beginning */
    node->next = l->head;
    l->head = node;
    l->count++;

    if (l->count > MAX_LOG)
        log_delete_last(l);
    return 1;
}

void log_delete_last(LogList *l)
{
    LogNode *temp, *prev;

    if (l->head == NULL)
        return;                                /* underflow: nothing to delete */

    if (l->head->next == NULL) {               /* only one node */
        free(l->head);
        l->head = NULL;
        l->count = 0;
        return;
    }

    /* go till the second last node */
    prev = l->head;
    temp = l->head->next;
    while (temp->next != NULL) {
        prev = temp;
        temp = temp->next;
    }
    prev->next = NULL;
    free(temp);
    l->count--;
}

void log_free(LogList *l)
{
    LogNode *temp;
    while (l->head != NULL) {
        temp = l->head;
        l->head = l->head->next;
        free(temp);
    }
    l->count = 0;
}
