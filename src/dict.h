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

#ifndef LIBTWINE_DICT_H
#define LIBTWINE_DICT_H

#include "twine.h"
#include "conn.h"


/* Underlying hash table used by `twine__dict`. */
struct twine__dict_table {
    /* Array of hash buckets. */
    struct twine_conn ** entries;

    /* Size of the `entries` array; always a power of two. */
    uint32_t size;

    /* Key mask; always `size-1`. */
    uint32_t mask;
};


/* Dict instances are hash maps storing `twine_conn` structs keyed by their
 * local connection cookies. */
struct twine__dict {
    /* The dict's two hash tables. The second hash table is only used during
     * the process of resizing the underlying storage. */
    struct twine__dict_table tables[2];

    /* If non-zero, indicates the next bucket index to be moved from the first
     * hash table to the second. It then follows that all bucket indexes which
     * are `< split` have already been moved. */
    uint32_t split;

    /* Seed material for the key hashing function. */
    uint64_t seed[2];

    /* Number of entries currently stored in the dict. */
    uint64_t count;

    /* Socket configuration object, used here only for its memory management
     * functions. */
    struct twine_conf * conf;
};


/* Initialize a dict instance. The call returns zero or success, or
 * TWINE_ENOMEM if a necessary allocation failed. */
int twine__dict_init(struct twine__dict * dict, struct twine_conf * conf, uint64_t seed[2]);

/* Free the dict's internal hash table(s). */
void twine__dict_destroy(struct twine__dict * dict);


/* Look up a connection in the dict by its local connection cookie. The
 * returned pointer will be NULL if no matching entry could be found. */
struct twine_conn * twine__dict_find(struct twine__dict * dict, uint64_t cookie);

/* Add a connection entry to the dict. The call will return zero on success,
 * or TWINE_ENOMEM if the dict is large enough that the underlying hash table
 * should be resized but the allocation failed.
 *
 * The implementation makes the assumption that local connection cookies are
 * unique, and that the same connection won't be inserted twice. */
int twine__dict_add(struct twine__dict * dict, struct twine_conn * conn);

/* Remove a connection entry from the dict. */
int twine__dict_remove(struct twine__dict * dict, struct twine_conn * conn);


#endif
