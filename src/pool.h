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
