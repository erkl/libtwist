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
