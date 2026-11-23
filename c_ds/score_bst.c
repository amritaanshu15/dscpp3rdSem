/*
 * score_bst.c  -  BST for leaderboard (all functions are recursive)
 */
#include <stdlib.h>
#include <string.h>
#include "score_bst.h"

ScoreNode *bst_insert(ScoreNode *root, const char *name, int score, int days)
{
    if (root == NULL) {
        ScoreNode *node = (ScoreNode *)malloc(sizeof(ScoreNode));
        if (node == NULL)
            return NULL;
        strncpy(node->data.name, name, NAME_LEN - 1);
        node->data.name[NAME_LEN - 1] = '\0';
        node->data.score = score;
        node->data.days = days;
        node->left = NULL;
        node->right = NULL;
        return node;
    }

    if (score > root->data.score)
        root->right = bst_insert(root->right, name, score, days);
    else
        root->left = bst_insert(root->left, name, score, days);
    return root;
}

/* reverse inorder: right subtree, this node, left subtree */
void bst_top(const ScoreNode *root, ScoreRec out[], int max, int *count)
{
    if (root == NULL || *count >= max)
        return;

    bst_top(root->right, out, max, count);
    if (*count < max) {
        out[*count] = root->data;
        (*count)++;
    }
    bst_top(root->left, out, max, count);
}

int bst_count(const ScoreNode *root)
{
    if (root == NULL)
        return 0;
    return 1 + bst_count(root->left) + bst_count(root->right);
}

int bst_height(const ScoreNode *root)
{
    int lh, rh;
    if (root == NULL)
        return 0;
    lh = bst_height(root->left);
    rh = bst_height(root->right);
    return 1 + (lh > rh ? lh : rh);
}

/* postorder: children first, then the node itself */
void bst_free(ScoreNode *root)
{
    if (root == NULL)
        return;
    bst_free(root->left);
    bst_free(root->right);
    free(root);
}
