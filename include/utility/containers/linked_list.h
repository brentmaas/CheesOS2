#ifndef _CHEESOS2_UTILITY_CONTAINERS_LINKED_LIST_H
#define _CHEESOS2_UTILITY_CONTAINERS_LINKED_LIST_H

// A node in a linked list. This structure can be embedded inside other types, of which the data
// can then be found by using the `CONTAINER_OF` macro.
struct linked_list_node {
    // The next node in this linked list.
    struct linked_list_node* next;

    // The previous node in this linked list.
    struct linked_list_node* prev;
};

void linked_list_insert_after(struct linked_list_node* node, struct linked_list_node* new);

void linked_list_remove(struct linked_list_node* node);

#endif
