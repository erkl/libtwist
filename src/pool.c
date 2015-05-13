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

#include "include/twine.h"
#include "src/pool.h"


/* Initialize an object pool. */
void twine__pool_init(struct twine__pool * pool, struct twine_conf * conf) {
    pool->head = NULL;
    pool->count = 0;
    pool->conf = conf;
}


/* Free all but `keep` objects from the pool. If the pool doesn't contain more
 * than `keep` objects the function call does nothing. */
void twine__pool_cull(struct twine__pool * pool, unsigned int keep) {
    void * next;

    /* Free objects until only `keep` are left. */
    while (pool->count > keep) {
        next = *((void **) pool->head);
        pool->conf->free(pool->head);
        pool->head = next;
        pool->count--;
    }
}


/* Grab an object from the pool or, if the pool is empty, allocate a new one. */
void * twine__pool_alloc(struct twine__pool * pool) {
    void * obj;

    if (pool->head != NULL) {
        obj = pool->head;
        pool->head = *((void **) obj);
        pool->count--;
    } else {
        obj = pool->conf->malloc(POOL_OBJECT_SIZE);
    }

    return obj;
}


/* Recycle an object back into the pool. If the pool is already at capacity,
 * the object will be freed. */
void twine__pool_free(struct twine__pool * pool, void * obj) {
    /* Doesn't hurt to be paranoid about these things. */
    if (pool->count == UINT_MAX) {
        pool->conf->free(obj);
        return;
    }

    /* Link to the previous object by treating the very first bytes of this
     * object as a `void *` pointer. */
    *((void **) obj) = pool->head;

    pool->head = obj;
    pool->count++;
}
