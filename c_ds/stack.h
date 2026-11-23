/*
 * stack.h  -  stack using array
 * DS topic: Stack, push, pop, array representation of stack
 *
 * Used in pathfind.c: BFS gives the path backwards (rabbit -> fox),
 * we push it on a stack and pop to get it in the right order (fox -> rabbit).
 */
#ifndef STACK_H
#define STACK_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STACK_SIZE (ROWS * COLS)

typedef struct {
    Cell items[STACK_SIZE];
    int top;                       /* -1 means empty */
} Stack;

void st_init(Stack *s);
int  st_isEmpty(const Stack *s);
int  st_push(Stack *s, Cell c);    /* 1 = ok, 0 = overflow  */
int  st_pop(Stack *s, Cell *out);  /* 1 = ok, 0 = underflow */
int  st_size(const Stack *s);

#ifdef __cplusplus
}
#endif

#endif
