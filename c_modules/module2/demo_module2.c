#include <stdio.h>
#include "pop_list.h"

int main(void)
{
    PopList list;
    list_init(&list);

    list_insert(&list, 1, "Grass", TYPE_PLANT, 500);
    list_insert(&list, 2, "Deer",  TYPE_HERBIVORE, 60);
    list_insert(&list, 3, "Tiger", TYPE_CARNIVORE, 6);
    list_display(&list);
    printf("Count = %d\n", list_count(&list));

    list_update_population(&list, 2, 45);
    printf("After updating Deer to 45:\n");
    list_display(&list);

    list_delete(&list, 3);
    printf("After deleting Tiger:\n");
    list_display(&list);
    printf("Search id 2: %s\n", list_search(&list, 2) ? "found" : "not found");

    list_free(&list);
    return 0;
}
