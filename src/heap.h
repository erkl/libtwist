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

#ifndef LIBTWINE_HEAP_H
#define LIBTWINE_HEAP_H


/* This is a simple min-heap for storing connections ordered by when their
 * next time-based event is scheduled to occur. */
struct twine__heap {
    /* Underlying storage array. */
    struct twine_conn ** entries;

    /* Number of connections currently stored in the heap. */
    uint32_t count;

    /* The underlying storage array's maximum capacity. */
    uint32_t size;

    /* Socket configuration. */
    struct twine_conf * conf;
};


/* Initialize the heap structure. Returns TWINE_ENOMEM if a necessary
 * allocation failed, otherwise TWINE_OK. */
int twine__heap_init(struct twine__heap * heap, struct twine_conf * conf);

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
