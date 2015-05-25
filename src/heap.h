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

#ifndef LIBTWINE_HEAP_H
#define LIBTWINE_HEAP_H

#include "include/twine.h"


/* This is a simple min-heap for storing connections ordered by when their
 * next time-based event is scheduled to occur. */
struct twine__heap {
    /* Underlying storage array. */
    struct twine_conn ** entries;

    /* Number of connections currently stored in the heap. */
    uint32_t count;

    /* The underlying storage array's maximum capacity. */
    uint32_t size;
};


/* Initialize the heap structure. Returns TWINE_ENOMEM if a necessary
 * allocation failed, otherwise TWINE_OK. */
int twine__heap_init(struct twine__heap * heap);

/* Free the heap's underlying storage. */
void twine__heap_destroy(struct twine__heap * heap);


/* Get the heap's top-most connection, or NULL if the heap is empty. */
struct twine_conn * twine__heap_peek(struct twine__heap * heap);

/* Push a new connection onto the heap. */
int twine__heap_add(struct twine__heap * heap, struct twine_conn * conn);

/* Remove a connection from the heap. */
int twine__heap_remove(struct twine__heap * heap, struct twine_conn * conn);


/* Re-establish heap ordering after a particular connection's `next_tick`
 * value has changed. */
void twine__heap_fix(struct twine__heap * heap, struct twine_conn * conn);


#endif
