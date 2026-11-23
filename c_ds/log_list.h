/*
 * log_list.h  -  singly linked list of game messages
 * DS topic: Linked list, insertion at beginning, deletion at end, malloc/free
 *
 * New message -> inserted at the HEAD (so newest is shown first).
 * If more than MAX_LOG messages -> the LAST node is deleted.
 */
#ifndef LOG_LIST_H
#define LOG_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_LOG  8
#define MSG_LEN  100

typedef struct LogNode {
    char text[MSG_LEN];
    struct LogNode *next;          /* self referential pointer */
} LogNode;

typedef struct {
    LogNode *head;
    int count;
} LogList;

void log_init(LogList *l);
int  log_add(LogList *l, const char *text);   /* 1 = ok, 0 = malloc failed (overflow) */
void log_delete_last(LogList *l);
void log_free(LogList *l);                     /* free every node */

#ifdef __cplusplus
}
#endif

#endif
