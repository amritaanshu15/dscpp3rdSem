#include <stdio.h>
#include "graph.h"

void graph_init(Graph *g)
{
    memset(g, 0, sizeof(Graph));
}

static int valid(const Graph *g, int n)
{
    return n >= 0 && n < MAX_NODES && g->active[n];
}

int graph_add_node(Graph *g, int node, const char *name)
{
    if (node < 0 || node >= MAX_NODES) return 0;
    g->active[node] = 1;
    eco_copy_name(g->names[node], name);
    return 1;
}

int graph_remove_node(Graph *g, int node)
{
    int i;
    if (!valid(g, node)) return 0;
    for (i = 0; i < MAX_NODES; i++) {          
        g->adj[node][i] = 0;
        g->adj[i][node] = 0;
    }
    g->active[node] = 0;
    return 1;
}

int graph_add_edge(Graph *g, int from, int to)
{
    if (!valid(g, from) || !valid(g, to) || from == to) return 0;
    if (g->adj[from][to]) return 0;
    g->adj[from][to] = 1;
    return 1;
}

int graph_remove_edge(Graph *g, int from, int to)
{
    if (!valid(g, from) || !valid(g, to) || !g->adj[from][to]) return 0;
    g->adj[from][to] = 0;
    return 1;
}

int graph_has_edge(const Graph *g, int from, int to)
{
    return valid(g, from) && valid(g, to) && g->adj[from][to];
}

void graph_display(const Graph *g)
{
    int i, j;
    for (i = 0; i < MAX_NODES; i++) {
        if (!g->active[i]) continue;
        printf("%-10s ->", g->names[i]);
        for (j = 0; j < MAX_NODES; j++)
            if (g->adj[i][j]) printf(" %s", g->names[j]);
        printf("\n");
    }
}

static void dfs_visit(const Graph *g, int node, int visited[], int order[], int *n)
{
    int next;
    visited[node] = 1;
    order[(*n)++] = node;
    for (next = 0; next < MAX_NODES; next++)
        if (g->adj[node][next] && !visited[next])
            dfs_visit(g, next, visited, order, n);
}

int graph_dfs(const Graph *g, int start, int order[])
{
    int visited[MAX_NODES] = {0};
    int n = 0;
    if (!valid(g, start)) return 0;
    dfs_visit(g, start, visited, order, &n);
    return n;
}

int graph_bfs(const Graph *g, int start, int order[])
{
    int visited[MAX_NODES] = {0};
    int queue[MAX_NODES];
    int head = 0, tail = 0, n = 0, cur, next;

    if (!valid(g, start)) return 0;
    visited[start] = 1;
    queue[tail++] = start;
    while (head < tail) {
        cur = queue[head++];                  
        order[n++] = cur;
        for (next = 0; next < MAX_NODES; next++)
            if (g->adj[cur][next] && !visited[next]) {
                visited[next] = 1;
                queue[tail++] = next;         
            }
    }
    return n;
}

static void comp_visit(const Graph *g, int node, int id, int comp[])
{
    int other;
    comp[node] = id;
    for (other = 0; other < MAX_NODES; other++)
        if (g->active[other] && comp[other] == -1 && (g->adj[node][other] || g->adj[other][node]))
            comp_visit(g, other, id, comp);
}

int graph_components(const Graph *g, int comp[])
{
    int i, groups = 0;
    for (i = 0; i < MAX_NODES; i++) comp[i] = -1;
    for (i = 0; i < MAX_NODES; i++)
        if (g->active[i] && comp[i] == -1)
            comp_visit(g, i, groups++, comp);
    return groups;
}
