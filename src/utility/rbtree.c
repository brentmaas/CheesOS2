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

static bool rb_node_is_black(struct rb_node* node) {
    return !rb_node_is_red(node);
}

static void rb_fix_insert(struct rb_tree* tree, struct rb_node* node) {
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
                    Note, (R) might also have been the other child of its parent for this case.
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

static void rb_swap_color(struct rb_node* a, struct rb_node* b) {
    bool color = a->is_black;
    a->is_black = b->is_black;
    b->is_black = color;
}

// Delete an inner node by swapping it with a leaf node.
// Returns the new node to delete (the leaf). Afterwards, `node` only
// has a single child.
static void rb_bst_swap(struct rb_node* node) {
    if (!node->left || !node->right) {
        // If the node had only one child to begin with, there is nothing to do here.
        return;
    }

    struct rb_node* parent = node->parent;

    // Look for the inorder successor, the node with the next larger value.
    // This is easily found from the left-most descendant of the right child.
    struct rb_node* replacement = node->right;
    if (!replacement->left) {
        // Special case: the replacement node is a direct child of the original node.
        // In this case we cannot simply swap the pointers.
        // We are now in the following case, where b and parent are optional (but a is not):
        /*
              parent?              parent?
              |                    |
              node                 replacement
             / \             =>   / \
            a   replacement      a   node
                 \                    \
                  b?                   b?
        */
        struct rb_node* a = node->left;
        struct rb_node* b = replacement->right;

        // First, fix up a, b and parent.
        // Fix child of parent.
        if (parent) {
            if (parent->left == node)
                parent->left = replacement;
            else
                parent->right = replacement;
        }

        // Fix parent of a.
        a->parent = replacement;

        // Fix parent of b.
        if (b) {
            b->parent = node;
        }

        // Fix up the node and replacement themselves.
        replacement->parent = parent;
        node->parent = replacement;

        replacement->left = a;
        replacement->right = node;

        node->left = NULL;
        node->right = b;

        rb_swap_color(node, replacement);
        return;
    }

    while (replacement->left) {
        replacement = replacement->left;
    }

    // Swap node and replacement, but this time we're sure that both nodes
    // are not in direct relation with eachother, and so we can just swap the pointers.
    // We have the following situation:
    /*
          parent?       replacement->parent
          |            /
          node        replacement
         / \           \
        a   b           c?
    */

    // Fix parents.
    if (parent) {
        if (parent->left == node)
            parent->left = replacement;
        else
            parent->right = replacement;
    }

    // `replacement` always has a parent, and is always the left child of its parent.
    replacement->parent->left = node;

    // Fix children.
    // Note: node->left and node->right exist, but replacement->left does not.
    // replacement->right is optional.
    node->right->parent = replacement;
    node->left->parent = replacement;

    if (replacement->right)
        replacement->right->parent = node;

    // Fix nodes themselves.
    node->parent = replacement->parent;
    replacement->parent = parent;

    replacement->left = node->left;
    node->left = NULL;

    struct rb_node* tmp = node->right;
    node->right = replacement->right;
    replacement->right = tmp;

    rb_swap_color(node, replacement);
}

// Fix a double-black case. `node` must not be the root, and must have no children.
static void rb_fix_double_black(struct rb_tree* tree, struct rb_node* node) {
    while (node->parent) {
        struct rb_node* parent = node->parent;

        // Note: The sibling cannot be null at this point, otherwise the black-height would not be
        // the same for every node. This is because the null children of `node` also count towards the
        // black-height.
        struct rb_node* sibling = node == parent->left ? parent->right : parent->left;
        struct rb_node* close_nephew = node == parent->left ? sibling->left : sibling->right;
        struct rb_node* far_nephew = node == parent->left ? sibling->right : sibling->left;

        if (rb_node_is_black(sibling) && rb_node_is_black(close_nephew) && rb_node_is_black(far_nephew)) {
            /*
                Case 1: The sibling and children are black.
                This is resolved by painting the sibling red. If the parent was black, it becomes
                the double-black. Otherwise, it is paint black and iteration terminates.
                   X            B
                  / \          / \
                (B)  B   =>  (B)  R
                    / \          / \
                   B   B        B   B
            */
            sibling->is_black = false;
            if (!rb_node_is_black(parent)) {
                parent->is_black = true;
                return;
            }

            node = parent;
            continue;
        } else if (rb_node_is_red(sibling)) {
            /*
                Case 2: The sibling is red.
                This automatically means that the children of the sibling and the parent are black.
                This is resolved by first swapping the colors of the sibling and the parent,
                and then performing a rotation on the parent in the direction of the double-black.
                This also leads to a new double-black situation, so the iteration continues with
                the original sibling.
                   B            R            B
                  / \          / \          / \
                (B)  R   =>  (B)  B   =>   R   B
                    / \          / \      / \
                   B   B        B   B   (B)  B
            */
            parent->is_black = false;
            sibling->is_black = true;

            if (node == parent->left)
                rb_rotate_left(tree, parent);
            else
                rb_rotate_right(tree, parent);

            node = sibling;
            continue;
        } else if (rb_node_is_black(far_nephew)) {
            /*
                Case 3: Sibling is black, far nephew is black, and close nephew is red.
                This situation is resolved by first swapping the color of the close
                nephew with the sibling, and then performing a rotation at the sibling node
                in the opposite direction of the DB node, and continuing to case 4 below.
                   X            X           X
                  / \          / \         / \
                (B)  B   =>  (B)  R   => (B)  B
                    / \          / \           \
                   R   B        B   B           R
                                                 \
                                                  B
            */
            sibling->is_black = false;
            close_nephew->is_black = true;
            if (node == parent->left)
                rb_rotate_right(tree, sibling);
            else
                rb_rotate_left(tree, sibling);

            // Don't forget to update the pointers after the rotation here.
            far_nephew = sibling;
            sibling = close_nephew;
            close_nephew = node == parent->left ? sibling->left : sibling->right;
        }

        /*
            Case 4: Sibling is black, far nephew is red.
            This case is resolved as follows:
            - First, the color of the parent is swapped with the color of the sibling.
            - A rotation is performed on the parent in the direction of the double-black.
            - Original far nephew is set to black.
            - Iteration terminates.
               X            B           X          X
              / \          / \         / \        / \
            (B)  B   =>  (B)  X   =>  B   R  =>  B   B
                / \          / \     / \        / \
               X   R        X   R  (B)  X     (B)  X
        */
        sibling->is_black = parent->is_black;
        parent->is_black = true;
        far_nephew->is_black = true;
        if (node == parent->left)
            rb_rotate_left(tree, parent);
        else
            rb_rotate_right(tree, parent);
        return;
    }
}

void rb_init(struct rb_tree* tree, rb_node_cmp_fn cmp) {
    tree->root = NULL;
    tree->num_nodes = 0;
    tree->cmp = cmp;
}

void rb_insert(struct rb_tree* tree, struct rb_node* node) {
    node->left = NULL;
    node->right = NULL;
    ++tree->num_nodes;

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
        current = tree->cmp(node, current) < 0 ? current->left : current->right;
    }

    node->parent = parent;
    // TODO: maybe save a comparison here?
    if (tree->cmp(node, parent) < 0) {
        parent->left = node;
    } else {
        parent->right = node;
    }

    rb_fix_insert(tree, node);
}

void rb_delete(struct rb_tree* tree, struct rb_node* node) {
    --tree->num_nodes;
    // Special case where `node` is the root of the tree and it doesn't have any children.
    if (node == tree->root && !node->left && !node->right) {
        tree->root = NULL;
        return;
    }

    // If the node is an inner node (including the root), replace it with a leaf node.
    // Afterward, `node` has at most one child, which may be either left or right.
    rb_bst_swap(node);

    struct rb_node* parent = node->parent;

    // At this point:
    // - if `node` has exactly one child the child is red.
    // - if `node` is red, it has no children.
    if (rb_node_is_red(node)) {
        // Simply remove the node.
        // Because the node is red, its parent is valid, and can never be the root.
        if (node == parent->left)
            parent->left = NULL;
        else
            parent->right = NULL;
        return;
    }

    // At this point:
    // - `node` is black.
    // - `node` might have one or no children.
    // - if `node` has one child, it is red. In this case, we can replace `node` with the child and
    //   repaint the child black.
    // - `node` might also still be the root.
    if (node->left || node->right) {
        struct rb_node* replacement = node->left ? node->left : node->right;
        // We have the following case (or the symmetric case):
        // Note that `node` might become the new root, in this case.
        /*
                parent?         parent?
                |               |
                node            replacement
               /           =>  / \
              replacement
             / \
        */

        replacement->is_black = true;
        replacement->parent = parent;

        if (parent) {
            if (node == parent->left)
                parent->left = replacement;
            else
                parent->right = replacement;
        } else {
            tree->root = replacement;
        }

        return;
    }

    // At this point:
    // - `node` is black.
    // - `node` has no children.
    // - `node` is not the root (this case is handled at the top of the function).
    // This is the 'double black' case.
    rb_fix_double_black(tree, node);

    if (node == parent->left)
        parent->left = NULL;
    else
        parent->right = NULL;
}

struct int_node {
    struct rb_node node;
    int value;
};

struct rb_node* rb_find(struct rb_tree* tree, struct rb_node* node) {
    return rb_find_by(tree, (rb_find_cmp_fn) tree->cmp, node);
}

struct rb_node* rb_find_by(struct rb_tree* tree, rb_find_cmp_fn cmp, void* value) {
    struct rb_node* current = tree->root;
    while (current) {
        int order = cmp(value, current);
        if (order == 0) {
            return current;
        } else if (order < 0) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    return NULL;
}

void rb_iterator_init(struct rb_iterator* it, struct rb_tree* tree) {
    it->next = tree->root;
    it->node = NULL;
}

void rb_iterator_init_at(struct rb_iterator* it, struct rb_node* start) {
    it->next = start;
    // We want to make this node be the next node that is visited, and after ward
    // we want the iterator to advance to the right child. This means that the previous
    // node is either the left child if it exists, or the parent of not.
    it->node = start->left ? start->left : start->parent;
}

bool rb_iterator_next(struct rb_iterator* it) {
    if (!it->next) {
        it->node = NULL;
        return false;
    }

    struct rb_node* current = it->next;
    struct rb_node* prev = it->node;

    while (true) {
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
            } else {
                current = current->parent;
            }
            break;
        } else if (!current->parent) {
            // Came from right child and would go to parent, but it doesn't exist.
            // We're at the end.
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

int int_node_cmp(struct rb_node* lhs, struct rb_node* rhs) {
    return ((struct int_node*) lhs)->value - ((struct int_node*) rhs)->value;
}

void test_iterate_cb(struct rb_node* node) {
    log_debug("Visiting node with value %i", ((struct int_node*) node)->value);
}

void rb_test() {
    struct int_node nodes[10];

    struct rb_tree tree;
    rb_init(&tree, int_node_cmp);

    for (size_t i = 0; i < 10; ++i) {
        nodes[i].value = (10 * (i + 1)) % 13;
        log_debug("Inserting node with value %u", nodes[i].value);
        rb_insert(&tree, &nodes[i].node);
    }

    struct rb_node* x = rb_find(&tree, (struct rb_node*) &nodes[1]);
    log_debug("Finding node with value %u", nodes[1].value);
    if (x)
        log_debug("Found node with value %u", ((struct int_node*) x)->value);
    else
        log_debug("No such node");

    for (size_t i = 0; i < 10; i += 2) {
        log_debug("Deleting node with value %u (current size: %u)", nodes[i].value, tree.num_nodes);
        rb_delete(&tree, &nodes[i].node);
    }

    struct rb_iterator it;
    rb_iterator_init(&it, &tree);
    while (rb_iterator_next(&it)) {
        log_debug("Visiting node with value %u", ((struct int_node*) it.node)->value);
    }
}
