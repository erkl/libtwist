/* Copyright (c) 2015, Erik Lundin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the copyright holder nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#ifndef LIBTWINE_BUFFER_H
#define LIBTWINE_BUFFER_H

#include "include/twine.h"
#include "src/pool.h"


/* A slab describes a chunk of memory used by a buffer. */
struct twine__buffer_slab {
    /* Start and end pointers, which will point to memory immediately after
     * the slab header itself. */
    uint8_t * start;
    uint8_t * end;

    /* Intrusive list pointer. */
    struct twine__buffer_slab * next;
};


/* This struct implements a dynamically growing and shrinking buffer, with
 * writes to the back and reads from the front. */
struct twine__buffer {
    /* Linked list of buffer slabs. */
    struct twine__buffer_slab * head;
    struct twine__buffer_slab * tail;

    /* Number of bytes currently stored in the buffer. */
    size_t size;

    /* Assigned memory pool. */
    struct twine__pool * pool;
};


/* Initialize the buffer's internal fields. */
void twine__buffer_init(struct twine__buffer * bufr, struct twine__pool * pool);

/* Discard all data and return the buffer's slabs to the object pool. */
void twine__buffer_clear(struct twine__buffer * bufr);

/* Write a chunk of data to the buffer. Unless a necessary memory allocation
 * fails (in which case no data is written and TWINE_ENOMEM is returned), the
 * full write is guaranteed to complete successfully. The `len` argument must
 * not be greater than SSIZE_MAX. */
ssize_t twine__buffer_write(struct twine__buffer * bufr, const uint8_t * buf, size_t len);

/* Read data from the buffer. This call can't fail, only return 0 when the
 * buffer is empty. The `len` argument must not be greater than SSIZE_MAX. */
ssize_t twine__buffer_read(struct twine__buffer * bufr, uint8_t * buf, size_t len);

/* Get the number of bytes of data currently stored in the buffer. */
size_t twine__buffer_size(struct twine__buffer * bufr);


#endif
