#ifndef _CHEESOS2_UTILITY_RBTREE_H
#define _CHEESOS2_UTILITY_RBTREE_H

#include <stdbool.h>
#include <stddef.h>

// A structure representing a node of a red-black tree.
// Nodes are supposed to the tree when they are inserted, and no memory is owned by the tree.
// Typically, nodes should be embedded in other structures which carry node information. The
// node pointer that is passed in the comparison callback and node pointers returned from other
// functions can then be converted back into the original structure to obtain node data.
struct rb_node {
    struct rb_node* parent;
    struct rb_node* left;
    struct rb_node* right;
    bool is_black;
};

// A user-supplied node comparison function. This function should return:
// - A negative value when lhs < rhs.
// - 0 when lhs == rhs.
// - A positive value when lhs > rhs.
typedef int (*rb_node_cmp_fn)(struct rb_node* lhs, struct rb_node* rhs);

// A structure representing the red-black tree itself.
struct rb_tree {
    // The root node of the tree.
    struct rb_node* root;

    // The total nodes currently in the tree.
    size_t num_nodes;

    // The comparison function used to compare two nodes.
    rb_node_cmp_fn cmp;
};

// Initialize a red-black tree with some node comparison function `cmp`.
void rb_init(struct rb_tree* tree, rb_node_cmp_fn cmp);

// Insert a new node into the red-black tree.
void rb_insert(struct rb_tree* tree, struct rb_node* node);

// Delete a particular node from the red-black tree.
// `node` is the exact node that must be deleted, *not* a node that compares equal.
void rb_delete(struct rb_tree* tree, struct rb_node* node);

// Traverse the tree to find a node which compares equal to `node`.
// Returns `NULL` if the tree contained no such node.
struct rb_node* rb_find(struct rb_tree* tree, struct rb_node* node);

// User-supplied comparison function. This function should return:
// - A negative value when lhs < rhs.
// - 0 when lhs == rhs.
// - A positive value when lhs > rhs.
typedef int (*rb_find_cmp_fn)(void* lhs, struct rb_node* rhs);

// Find a node using a user-supplied node comparison function. This removes the need to construct a struct
// with a node in order to find a value.
struct rb_node* rb_find_by(struct rb_tree* tree, rb_find_cmp_fn cmp, void* value);

// A structure used for iterating (in in-order) over the nodes of a red-black tree.
// The current node can be accessed using `it->node`.
struct rb_iterator {
    struct rb_node* next;
    struct rb_node* node;
};

// Initialize a red-black tree node iterator, starting at the root of the tree.
void rb_iterator_init(struct rb_iterator* it, struct rb_tree* tree);

// Initialize a red-black tree node iterator, starting at a particular node. The first call to
// `rb_iterator_next` will return the starting node.
void rb_iterator_init_at(struct rb_iterator* it, struct rb_node* start);

// Advance the iterator to the next node in the tree. Returns `true` if the current node is
// valid, and `false` if the end has been reached.
bool rb_iterator_next(struct rb_iterator* it);

void rb_test();

#endif
