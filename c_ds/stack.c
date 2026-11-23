/*
 * stack.c  -  array stack
 */
#include "stack.h"

void st_init(Stack *s)
{
    s->top = -1;
}

int st_isEmpty(const Stack *s)
{
    return s->top == -1;
}

int st_push(Stack *s, Cell c)
{
    if (s->top == STACK_SIZE - 1)
        return 0;                  /* stack overflow */
    s->top++;
    s->items[s->top] = c;
    return 1;
}

int st_pop(Stack *s, Cell *out)
{
    if (st_isEmpty(s))
        return 0;                  /* stack underflow */
    *out = s->items[s->top];
    s->top--;
    return 1;
}

int st_size(const Stack *s)
{
    return s->top + 1;
}
