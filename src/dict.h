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

#ifndef LIBTWINE_DICT_H
#define LIBTWINE_DICT_H

#include "include/twine.h"
#include "src/conn.h"


/* Underlying hash table used by `twine__dict`. */
struct twine__dict_table {
    /* Array of hash table buckets. */
    struct twine_conn ** buckets;

    /* Size of the `buckets` array; always a power of two. */
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
    uint8_t seed[16];

    /* Number of entries currently stored in the dict. */
    uint64_t count;
};


/* Initialize a dict instance. The call returns zero or success, or
 * TWINE_ENOMEM if a necessary allocation failed. */
int twine__dict_init(struct twine__dict * dict, uint8_t seed[16]);

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
