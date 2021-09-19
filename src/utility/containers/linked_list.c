#include "utility/containers/linked_list.h"

void linked_list_insert_after(struct linked_list_node* node, struct linked_list_node* new) {
    new->next = node->next;
    new->prev = node;

    new->next->prev = new;

    node->next = new;
}

void linked_list_remove(struct linked_list_node* node) {
    struct linked_list_node* next = node->next;
    struct linked_list_node* prev = node->prev;

    if (next)
        next->prev = prev;

    if (prev)
        prev->next = next;
}
