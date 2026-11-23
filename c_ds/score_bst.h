/*
 * score_bst.h  -  high scores stored in a Binary Search Tree
 * DS topic: Binary search tree, insertion, traversal (inorder), recursion
 *
 * Rule: bigger score goes to the RIGHT, smaller or equal score to the LEFT.
 * Reverse inorder (right -> root -> left) gives scores from highest to lowest.
 */
#ifndef SCORE_BST_H
#define SCORE_BST_H

#ifdef __cplusplus
extern "C" {
#endif

#define NAME_LEN 20

typedef struct {
    char name[NAME_LEN];
    int  score;
    int  days;
} ScoreRec;

typedef struct ScoreNode {
    ScoreRec data;
    struct ScoreNode *left;
    struct ScoreNode *right;
} ScoreNode;

ScoreNode *bst_insert(ScoreNode *root, const char *name, int score, int days);
void       bst_top(const ScoreNode *root, ScoreRec out[], int max, int *count);  /* best 'max' scores */
int        bst_count(const ScoreNode *root);
int        bst_height(const ScoreNode *root);
void       bst_free(ScoreNode *root);                                            /* postorder free */

#ifdef __cplusplus
}
#endif

#endif
