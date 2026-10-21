#include <stdio.h>
#include "graph.h"

int main(void)
{
    Graph g;
    int order[MAX_NODES], i, n;

    graph_init(&g);
    graph_add_node(&g, 0, "Grass");
    graph_add_node(&g, 1, "Deer");
    graph_add_node(&g, 2, "Tiger");
    graph_add_node(&g, 3, "Rabbit");
    graph_add_node(&g, 4, "Fox");
    graph_add_edge(&g, 0, 1);  graph_add_edge(&g, 1, 2);   
    graph_add_edge(&g, 0, 3);  graph_add_edge(&g, 3, 4);   

    graph_display(&g);

    n = graph_dfs(&g, 0, order);
    printf("\nDFS from Grass: ");
    for (i = 0; i < n; i++) printf("%s ", g.names[order[i]]);

    n = graph_bfs(&g, 0, order);
    printf("\nBFS from Grass: ");
    for (i = 0; i < n; i++) printf("%s ", g.names[order[i]]);
    printf("\n");
    return 0;
}
