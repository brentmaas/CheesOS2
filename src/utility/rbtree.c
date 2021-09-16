#include "utility/rbtree.h"
#include "debug/assert.h"
#include "debug/log.h"

#include <stddef.h>

/* Perform a left-rotation:
      x            y
     / \          / \
    a   y   =>   x   c
       / \      / \
      b   c    a   b
*/
static void rb_rotate_left(struct rb_tree* tree, struct rb_node* x) {
    struct rb_node* y = x->right;

    // update b
    x->right = y->left;
    if (x->right) {
        x->right->parent = x;
    }

    y->parent = x->parent;
    if (!y->parent) {
        tree->root = y;
    } else if (y->parent->left == x) {
        y->parent->left = y;
    } else {
        y->parent->right = y;
    }

    // update y
    y->left = x;
    x->parent = y;
}

/* Perform a right-rotation:
        x          y
       / \        / \
      y   c  =>  a   x
     / \            / \
    a   b          b   c
*/
static void rb_rotate_right(struct rb_tree* tree, struct rb_node* x) {
    struct rb_node* y = x->left;

    // update b
    x->left = y->right;
    if (x->left) {
        x->left->parent = x;
    }

    // update parent
    y->parent = x->parent;
    if (!y->parent) {
        tree->root = y;
    } else if (y->parent->left == x) {
        y->parent->left = y;
    } else {
        y->parent->right = y;
    }

    // update y
    y->right = x;
    x->parent = y;
}

static bool rb_node_is_red(struct rb_node* node) {
    return node && !node->is_black;
}

static void rb_fix(struct rb_tree* tree, struct rb_node* node) {
    assert(node->parent);
    assert(!node->is_black);

    while (node != tree->root && !node->parent->is_black) {
        // Note: If the parent was red, it could not have been the root node, and so the grand parent
        // of `node` is also a valid node.
        struct rb_node* parent = node->parent;
        struct rb_node* grandparent = node->parent->parent;

        if (parent == grandparent->left) {
            // Note: uncle might be null
            struct rb_node* uncle = grandparent->right;
            if (rb_node_is_red(uncle)) {
                // Node: uncle cannot be null.
                /*
                    Case 1: the tree is transformed as follows:
                       B        R
                      / \      / \
                     R   R => B   B
                      \        \
                      (R)      (R)
                    Note, (R) might also have been the other child of it's parent for this case.
                    Afterward, we advance to the grandparent.
                */
                node->parent->is_black = true;
                uncle->is_black = true;
                grandparent->is_black = false;
                node = grandparent;
                continue;
            } else if (node == parent->right) {
                /*
                    Case 2: the tree is transformed as follows:
                       B         B
                      / \       / \
                     R   B => (R)   B
                      \       /
                      (R)    R
                    Afterward, we advance to the new child to fix that case.
                */
                node = parent;
                rb_rotate_left(tree, node);
            }
            /*
                Case 3: the tree is transformed as follows:
                     B      B
                    /      / \
                   R  => (R)  R
                  /
                (R)
            */
            parent->is_black = true;
            grandparent->is_black = false;
            rb_rotate_right(tree, grandparent);
            // Since the new parent is now black, we don't need to further fix up the tree and can exit.
            return;
        } else {
            // Symmetric cases
            // Note: uncle might be null
            struct rb_node* uncle = grandparent->right;
            if (rb_node_is_red(uncle)) {
                // Node: uncle cannot be null.
                // Case 1
                node->parent->is_black = true;
                uncle->is_black = true;
                grandparent->is_black = false;
                node = grandparent;
                continue;
            } else if (node == parent->left) {
                // Case 2
                node = parent;
                rb_rotate_right(tree, node);
            }
            // Case 3
            parent->is_black = true;
            grandparent->is_black = false;
            rb_rotate_left(tree, grandparent);
            return;
        }
    }

    // We might end up with a new red root node. This is not allowed, so color it black.
    node->is_black = true;
}

void rb_init(struct rb_tree* tree, rb_node_lt_fn lt) {
    tree->root = NULL;
    tree->lt = lt;
}

void rb_insert(struct rb_tree* tree, struct rb_node* node) {
    node->left = NULL;
    node->right = NULL;

    if (!tree->root) {
        node->parent = NULL;
        node->is_black = true;
        tree->root = node;
        return;
    }
    node->is_black = false;

    struct rb_node* current = tree->root;
    struct rb_node* parent;
    while (current) {
        parent = current;
        current = tree->lt(node, current) ? current->left : current->right;
    }

    node->parent = parent;
    // TODO: maybe save a comparison here?
    if (tree->lt(node, parent)) {
        parent->left = node;
    } else {
        parent->right = node;
    }

    rb_fix(tree, node);
}

void rb_delete(struct rb_tree* tree, struct rb_node* node) {
    // TODO
}

void rb_iterator_init(struct rb_tree* tree, struct rb_iterator* it) {
    it->next = tree->root;
    it->node = NULL;
}

bool rb_iterator_next(struct rb_iterator* it) {
    if (!it->next) {
        it->node = NULL;
        return false;
    }

    struct rb_node* current = it->next;
    struct rb_node* prev = it->node;

    while (current) {
        if (current->parent == prev) {
            // came from parent. Go to left child if exists, or visit and go to right or parent.
            prev = current;
            if (current->left) {
                current = current->left;
            } else if (current->right) {
                current = current->right;
                break;
            } else {
                current = current->parent;
                break;
            }
        } else if (current->left == prev) {
            // came from left child. Visit, and go to right child or parent.
            prev = current;
            if (current->right) {
                current = current->right;
                break;
            } else {
                current = current->parent;
            }
        } else if (!current->parent) {
            // Would go to parent, but it doesn't exist. We're at the end.
            it->next = NULL;
            it->node = NULL;
            return false;
        } else {
            // came from right child, go to parent.
            prev = current;
            current = current->parent;
        }
    }

    it->next = current;
    it->node = prev;
    return true;
}

struct int_node {
    struct rb_node node;
    int value;
};

bool int_node_lt(struct rb_node* lhs, struct rb_node* rhs) {
    return ((struct int_node*) lhs)->value < ((struct int_node*) rhs)->value;
}

void test_iterate_cb(struct rb_node* node) {
    log_debug("Visiting node with value %i", ((struct int_node*) node)->value);
}

void rb_test() {
    struct int_node nodes[10];

    struct rb_tree tree;
    rb_init(&tree, int_node_lt);

    for (size_t i = 0; i < 10; ++i) {
        nodes[i].value = (10 * (i + 1)) % 13;
        log_debug("Inserting node with value %u", nodes[i].value);
        rb_insert(&tree, &nodes[i].node);
    }

    struct rb_iterator it;
    rb_iterator_init(&tree, &it);
    while (rb_iterator_next(&it)) {
        log_debug("Visiting node with value %u", ((struct int_node*) it.node)->value);
    }
}
