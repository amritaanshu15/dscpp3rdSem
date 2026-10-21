#ifndef GRAPH_H
#define GRAPH_H

#include "../common/eco_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NODES MAX_ORGANISMS

typedef struct {
    int  active[MAX_NODES];
    char names[MAX_NODES][NAME_LEN];
    int  adj[MAX_NODES][MAX_NODES];
} Graph;

void graph_init(Graph *g);
int  graph_add_node(Graph *g, int node, const char *name);
int  graph_remove_node(Graph *g, int node);
int  graph_add_edge(Graph *g, int from, int to);
int  graph_remove_edge(Graph *g, int from, int to);
int  graph_has_edge(const Graph *g, int from, int to);
void graph_display(const Graph *g);

int  graph_dfs(const Graph *g, int start, int order[]);
int  graph_bfs(const Graph *g, int start, int order[]);

int  graph_components(const Graph *g, int comp[]);

#ifdef __cplusplus
}
#endif
#endif
