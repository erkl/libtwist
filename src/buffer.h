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

#ifndef LIBTWIST_BUFFER_H
#define LIBTWIST_BUFFER_H

#include "include/twist.h"
#include "src/pool.h"


/* A slab describes a chunk of memory used by a buffer. */
struct twist__buffer_slab {
    /* Start and end pointers, which will point to memory immediately after
     * the slab header itself. */
    uint8_t * start;
    uint8_t * end;

    /* Intrusive list pointer. */
    struct twist__buffer_slab * next;
};


/* This struct implements a dynamically growing and shrinking buffer, with
 * writes to the back and reads from the front. */
struct twist__buffer {
    /* Linked list of buffer slabs. */
    struct twist__buffer_slab * head;
    struct twist__buffer_slab * tail;

    /* Number of bytes currently stored in the buffer. */
    size_t size;

    /* Assigned memory pool. */
    struct twist__pool * pool;
};


/* Initialize the buffer's internal fields. */
void twist__buffer_init(struct twist__buffer * bufr, struct twist__pool * pool);

/* Discard all data and return the buffer's slabs to the object pool. */
void twist__buffer_clear(struct twist__buffer * bufr);

/* Write a chunk of data to the buffer. Unless a necessary memory allocation
 * fails (in which case no data is written and TWIST_ENOMEM is returned), the
 * full write is guaranteed to complete successfully. The `len` argument must
 * not be greater than SSIZE_MAX. */
ssize_t twist__buffer_write(struct twist__buffer * bufr, const uint8_t * buf, size_t len);

/* Read data from the buffer. This call can't fail, only return 0 when the
 * buffer is empty. The `len` argument must not be greater than SSIZE_MAX. */
ssize_t twist__buffer_read(struct twist__buffer * bufr, uint8_t * buf, size_t len);

/* Get the number of bytes of data currently stored in the buffer. */
size_t twist__buffer_size(struct twist__buffer * bufr);


#endif
