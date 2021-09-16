#ifndef _CHEESOS2_UTILITY_RBTREE_H
#define _CHEESOS2_UTILITY_RBTREE_H

#include <stdbool.h>

struct rb_node {
    struct rb_node* parent;
    struct rb_node* left;
    struct rb_node* right;
    bool is_black;
};

typedef bool (*rb_node_lt_fn)(struct rb_node* a, struct rb_node* b);

struct rb_tree {
    struct rb_node* root;
    rb_node_lt_fn lt;
};

void rb_init(struct rb_tree* tree, rb_node_lt_fn lt);

void rb_insert(struct rb_tree* tree, struct rb_node* node);

void rb_delete(struct rb_tree* tree, struct rb_node* node);

struct rb_iterator {
    struct rb_node* next;
    struct rb_node* node;
};

void rb_iterator_init(struct rb_tree* tree, struct rb_iterator* it);

bool rb_iterator_next(struct rb_iterator* it);

void rb_test();

#endif
