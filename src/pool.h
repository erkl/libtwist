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

#ifndef LIBTWINE_POOL_H
#define LIBTWINE_POOL_H

#include "include/twine.h"


/* Fixed size of pooled objects. This must be greater than MAX_PACKET_SIZE +
 * sizeof(struct twine__packet), and rounding it up to 2^10 + 2^9 should make
 * the `malloc` implementation's life a little bit easier. */
#define POOL_OBJECT_SIZE  1536


/* The `twine__pool` struct implements a simple last-in-first-out pool of
 * fixed-size blocks of memory. It doesn't free any objects on its own accord,
 * instead the user is expected to use `twine__pool_cull`. */
struct twine__pool {
    /* Linked list of free objects. */
    void * head;

    /* Number of free objects in the pool. */
    unsigned int count;

    /* Socket configuration object. */
    struct twine_conf * conf;
};


/* Initialize an object pool. */
void twine__pool_init(struct twine__pool * pool, struct twine_conf * conf);

/* Free all but `keep` objects from the pool. If there are already fewer than
 * `keep + 1` objects in the pool the function call does nothing. */
void twine__pool_cull(struct twine__pool * pool, unsigned int keep);

/* Grab an object from the pool or, if the pool is empty, allocate a new one. */
void * twine__pool_alloc(struct twine__pool * pool);

/* Recycle an object back into the pool. If the pool is already at capacity,
 * the object will be freed. */
void twine__pool_free(struct twine__pool * pool, void * obj);


#endif
