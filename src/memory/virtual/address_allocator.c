#include "memory/virtual/address_allocator.h"
#include "utility/container_of.h"

#include "debug/assert.h"
#include "debug/log.h"

static int aa_cmp_base(struct rb_node* lhs, struct rb_node* rhs) {
    return page_range_cmp_size(
        &CONTAINER_OF(struct address_allocator_hole, by_base_node, lhs)->range,
        &CONTAINER_OF(struct address_allocator_hole, by_base_node, rhs)->range
    );
}

static int aa_cmp_size(struct rb_node* lhs, struct rb_node* rhs) {
    return page_range_cmp_size(
        &CONTAINER_OF(struct address_allocator_hole, by_base_node, lhs)->range,
        &CONTAINER_OF(struct address_allocator_hole, by_base_node, rhs)->range
    );
}

static int aa_find_by_address(void* rhs, struct rb_node* lhs) {
    return page_range_cmp_address(
        (pageaddr_t) rhs,
        &CONTAINER_OF(struct address_allocator_hole, by_base_node, rhs)->range
    );
}

static int aa_find_by_size(void* rhs, struct rb_node* lhs) {
    size_t size = (size_t) rhs;
    struct address_allocator_hole* hole = CONTAINER_OF(struct address_allocator_hole, by_size_node, lhs);

    if (size == hole->range.size)
        return 0;
    else if (size < hole->range.size)
        return -1;
    return 1;
}

void address_allocator_init(struct address_allocator* aa, struct page_range range) {
    rb_init(&aa->holes_by_base, aa_cmp_base);
    rb_init(&aa->holes_by_size, aa_cmp_size);

    aa->scratch.range = range;
    rb_insert(&aa->holes_by_base, &aa->scratch.by_base_node);
    rb_insert(&aa->holes_by_size, &aa->scratch.by_size_node);
    log_debug("sizeof(aa node) = %zu", sizeof(struct address_allocator_hole));
}

bool address_allocator_alloc_anywhere(struct address_allocator* aa, pageaddr_t* base_out, size_t size) {
    assert(size > 0);

    // First, attempt to find a suitable node.
    struct rb_node* node = rb_find_smallest_not_below_by(&aa->holes_by_size, aa_find_by_size, (void*) (size - 1));
    if (!node)
        return false;

    struct address_allocator_hole* hole = CONTAINER_OF(struct address_allocator_hole, by_size_node, node);

    assert(hole->range.size >= size);
    *base_out = hole->range.base;

    if (hole->range.size == size) {
        // No need to carve, just remove the nodes.
        rb_delete(&aa->holes_by_base, &hole->by_base_node);
        rb_delete(&aa->holes_by_size, &hole->by_size_node);

        return true;
    }

    hole->range.base += size;
    hole->range.size -= size;

    // The node for the by base tree does not need to be deleted at this point; even if the address
    // changes, as long as the new range is not outside the old range, all nodes will compare the same
    // to the new values.
    // The by size node needs to be re-inserted, though.
    rb_delete(&aa->holes_by_size, &hole->by_size_node);
    rb_insert(&aa->holes_by_size, &hole->by_size_node);

    return true;
}

bool address_allocator_alloc_fixed(struct address_allocator* aa, struct page_range range) {
    return false;
}

void address_allocator_free(struct address_allocator* aa, struct page_range range) {

}

void address_allocator_dump(struct address_allocator* aa) {
    log_debug("Hole list:");
    struct rb_node* node = rb_first_node(&aa->holes_by_base);
    while (node) {
        struct address_allocator_hole* hole = CONTAINER_OF(struct address_allocator_hole, by_base_node, node);
        log_debug(
            "%p, %u KiB (%zu pages)",
            (void*) (hole->range.base * PAGE_SIZE),
            hole->range.size * (PAGE_SIZE / 1024),
            hole->range.size
        );

        node = rb_next_node(node);
    }
}
