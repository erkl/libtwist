/* Copyright (c) 2015, Erik Lundin.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE. */

#include "src/conn.h"
#include "src/heap.h"
#include "src/mem.h"


/* Minimum (default) and maximum hash table sizes. The maximum size may seem
 * arbitrary, but it's the highest we can go while being sure multiplication
 * by 8 (size of 64-bit pointers) won't overflow uint32_t. */
#define MIN_HEAP_SIZE  (1 << 6)
#define MAX_HEAP_SIZE  (1 << 28)


/* Static functions. */
static int resize(struct twist__heap * heap, uint32_t size);
static void up(struct twist__heap * heap, uint32_t index);
static void down(struct twist__heap * heap, uint32_t index);
static int less(struct twist__heap * heap, uint32_t i, uint32_t j);
static void swap(struct twist__heap * heap, uint32_t i, uint32_t j);


/* Initialize the heap structure. Returns TWIST_ENOMEM if a necessary
 * allocation failed, otherwise TWIST_OK. */
int twist__heap_init(struct twist__heap * heap) {
    struct twist__conn ** entries;

    /* Allocate the initial storage array. */
    entries = malloc(MIN_HEAP_SIZE * sizeof(struct twist__conn *));
    if (entries == NULL)
        return TWIST_ENOMEM;

    /* Update the heap's internal fields. */
    heap->entries = entries;
    heap->count = 0;
    heap->size = MIN_HEAP_SIZE;

    return TWIST_OK;
}


/* Free the heap's underlying storage. */
void twist__heap_clear(struct twist__heap * heap) {
    free(heap->entries);
}


/* Grab a pointer to the heap's top-most connection, or NULL if the heap
 * is empty. */
struct twist__conn * twist__heap_peek(struct twist__heap * heap) {
    return (heap->count > 0 ? heap->entries[0] : NULL);
}


/* Push a new connection onto the heap. */
int twist__heap_add(struct twist__heap * heap, struct twist__conn * conn) {
    int ret;

    /* If the underlying array is already full, grow it. */
    if (heap->count == heap->size) {
        if (heap->size == MAX_HEAP_SIZE)
            return TWIST_ENOMEM;

        ret = resize(heap, heap->size * 2);
        if (ret != TWIST_OK)
            return ret;
    }

    /* Append the new connection, then push it up towards the root entry until
     * heap ordering has been restored. */
    conn->heap_index = heap->count;
    heap->entries[heap->count] = conn;
    heap->count++;

    /* Restore heap ordering. */
    up(heap, conn->heap_index);

    return TWIST_OK;
}


/* Remove a connection from the heap. */
int twist__heap_remove(struct twist__heap * heap, struct twist__conn * conn) {
    uint32_t index;
    int ret;

    /* If 25% or less of the underlying array is in use, shrink it. */
    if ((heap->count - 1) <= (heap->size / 4) && heap->count > MIN_HEAP_SIZE) {
        ret = resize(heap, heap->size / 2);
        if (ret != TWIST_OK)
            return ret;
    }

    /* Swap the connection we're removing with the heap's last entry, then
     * decrement the entry count. */
    index = conn->heap_index;
    heap->count--;
    swap(heap, index, heap->count);

    /* Restore heap ordering. */
    down(heap, index);

    return TWIST_OK;
}


/* Re-establish the heap ordering after a particular entry's `next_tick` value
 * has changed. */
void twist__heap_fix(struct twist__heap * heap, struct twist__conn * conn) {
    uint32_t index;

    /* Load the original index, because `conn->heap_index` may be modified
     * during the `down` function call. */
    index = conn->heap_index;

    down(heap, index);
    up(heap, index);
}


/* Resize the heap's underlying storage. */
static int resize(struct twist__heap * heap, uint32_t size) {
    struct twist__conn ** entries;

    /* Allocate new storage. */
    entries = realloc(heap->entries, size * sizeof(struct twist__conn *));
    if (entries == NULL)
        return TWIST_ENOMEM;

    /* Update the relevant fields. */
    heap->entries = entries;
    heap->size = size;

    return TWIST_OK;
}


/* Select the entry at `index` and swap it continuously with its current parent
 * until the path from the heap's root to `index` is ordered again. */
static void up(struct twist__heap * heap, uint32_t index) {
    uint32_t parent;

    while (index != 0) {
        parent = (index - 1) / 2;

        /* Stop if the two entries are already in the correct order. */
        if (less(heap, parent, index))
            break;

        swap(heap, index, parent);
        index = parent;
    }
}


/* Select the entry at `index` and keep swapping it with the lesser of its
 * children until the subtree rooted at `index` is ordered again. */
static void down(struct twist__heap * heap, uint32_t index) {
    uint32_t left, right, child;

    for (;;) {
        left = 2 * index + 1;
        right = left + 1;

        /* As soon as we've reached a leaf node we're done. */
        if (left >= heap->count)
            break;

        /* Because this is a min-heap, we're interested in the lesser of the
         * two child nodes. */
        if (right >= heap->count || less(heap, left, right))
            child = left;
        else
            child = right;

        /* Stop if the two entries are already in the correct order. */
        if (less(heap, index, child))
            break;

        /* Swaperoo. */
        swap(heap, child, index);
        index = child;
    }
}


/* Compare two entries in the heap. Returns a non-zero value if the entry
 * at index `i` should be put in front of the entry at index `j`. */
static int less(struct twist__heap * heap, uint32_t i, uint32_t j) {
    struct twist__conn * x, * y;

    /* Grab the two connections. */
    x = heap->entries[i];
    y = heap->entries[j];

    /* Connections are ordered primarily by their `next_tick` fields. In case
     * of a tie, we use their local cookies to order them deterministically.
     * All zero or negative `next_tick` values are considered equal to each
     * other, but greater than any positive value. */
    if (x->next_tick <= 0) {
        if (y->next_tick > 0)
            return 0;
    } else {
        if (x->next_tick < y->next_tick || y->next_tick <= 0)
            return 1;
        if (x->next_tick > y->next_tick)
            return 0;
    }

    /* Use the local cookies, which will be unique, to break ties. */
    if (x->local_cookie < y->local_cookie)
        return 1;

    return 0;
}


/* Swap the position of entries in the heap. */
static void swap(struct twist__heap * heap, uint32_t i, uint32_t j) {
    struct twist__conn * x, * y;

    x = heap->entries[i];
    y = heap->entries[j];

    heap->entries[i] = y;
    heap->entries[j] = x;

    /* Remember to update the affected connections' `heap_index` fields. */
    x->heap_index = j;
    y->heap_index = i;
}
