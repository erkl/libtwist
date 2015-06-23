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

#include <limits.h>

#include "src/mem.h"
#include "src/pool.h"


/* Initialize an object pool. */
void twist__pool_init(struct twist__pool * pool) {
    pool->head = NULL;
    pool->count = 0;
}


/* Free all objects owned by the pool. */
void twist__pool_clear(struct twist__pool * pool) {
    twist__pool_cull(pool, 0);
}


/* Grab an object from the pool or, if the pool is empty, allocate a new one. */
void * twist__pool_alloc(struct twist__pool * pool) {
    void * obj;

    if (pool->head != NULL) {
        obj = pool->head;
        pool->head = *((void **) obj);
        pool->count--;
    } else {
        obj = malloc(POOL_OBJECT_SIZE);
    }

    return obj;
}


/* Recycle an object back into the pool. If the pool is already at capacity,
 * the object will be freed. */
void twist__pool_free(struct twist__pool * pool, void * obj) {
    /* Doesn't hurt to be paranoid about these things. */
    if (pool->count == UINT_MAX) {
        free(obj);
        return;
    }

    /* Link to the previous object by treating the very first bytes of this
     * object as a `void *` pointer. */
    *((void **) obj) = pool->head;

    pool->head = obj;
    pool->count++;
}


/* Free all but `keep` objects from the pool. If the pool doesn't contain more
 * than `keep` objects the function call does nothing. */
void twist__pool_cull(struct twist__pool * pool, unsigned int keep) {
    void * next;

    /* Free objects until only `keep` are left. */
    while (pool->count > keep) {
        next = *((void **) pool->head);
        free(pool->head);
        pool->head = next;
        pool->count--;
    }
}
