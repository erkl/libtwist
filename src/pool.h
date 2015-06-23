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

#ifndef LIBTWIST_POOL_H
#define LIBTWIST_POOL_H

#include "include/twist.h"


/* Fixed size of pooled objects. This must be greater than MAX_PACKET_SIZE +
 * sizeof(struct twist__packet), and rounding it up to 2^10 + 2^9 should make
 * the `malloc` implementation's life a little bit easier. */
#define POOL_OBJECT_SIZE  1536


/* The `twist__pool` struct implements a simple last-in-first-out pool of
 * fixed-size blocks of memory. It doesn't free any objects on its own accord,
 * instead the user is expected to use `twist__pool_cull`. */
struct twist__pool {
    /* Linked list of free objects. */
    void * head;

    /* Number of free objects in the pool. */
    unsigned int count;
};


/* Initialize an object pool. */
void twist__pool_init(struct twist__pool * pool);

/* Free all objects owned by the pool. */
void twist__pool_clear(struct twist__pool * pool);


/* Free all but `keep` objects from the pool. If there are already fewer than
 * `keep + 1` objects in the pool the function call does nothing. */
void twist__pool_cull(struct twist__pool * pool, unsigned int keep);

/* Grab an object from the pool or, if the pool is empty, allocate a new one. */
void * twist__pool_alloc(struct twist__pool * pool);

/* Recycle an object back into the pool. If the pool is already at capacity,
 * the object will be freed. */
void twist__pool_free(struct twist__pool * pool, void * obj);


#endif
