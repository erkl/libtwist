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

#ifndef LIBTWIST_HEAP_H
#define LIBTWIST_HEAP_H

#include "include/twist.h"


/* This is a simple min-heap for storing connections ordered by when their
 * next time-based event is scheduled to occur. */
struct twist__heap {
    /* Underlying storage array. */
    struct twist__conn ** entries;

    /* Number of connections currently stored in the heap. */
    uint32_t count;

    /* The underlying storage array's maximum capacity. */
    uint32_t size;
};


/* Initialize the heap structure. Returns TWIST_ENOMEM if a necessary
 * allocation failed, otherwise TWIST_OK. */
int twist__heap_init(struct twist__heap * heap);

/* Free the heap's underlying storage. */
void twist__heap_clear(struct twist__heap * heap);


/* Get the heap's top-most connection, or NULL if the heap is empty. */
struct twist__conn * twist__heap_peek(struct twist__heap * heap);

/* Push a new connection onto the heap. */
int twist__heap_add(struct twist__heap * heap, struct twist__conn * conn);

/* Remove a connection from the heap. */
int twist__heap_remove(struct twist__heap * heap, struct twist__conn * conn);


/* Re-establish heap ordering after a particular connection's `next_tick`
 * value has changed. */
void twist__heap_fix(struct twist__heap * heap, struct twist__conn * conn);


#endif
