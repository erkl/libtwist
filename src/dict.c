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

#include "dict.h"


/* Minimum (default) and maximum hash table sizes. The maximum size may seem
 * arbitrary, but it's the highest we can go while being sure multiplication
 * by 8 (size of 64-bit pointers) won't overflow. */
#define MIN_TABLE_SIZE  (1 << 6)
#define MAX_TABLE_SIZE  (1 << 28)


/* Static functions. */
static int maybe_resize(struct twine__dict * dict, int delta);
static void migrate_bucket(struct twine__dict * dict, uint32_t index);
static void migrate_buckets(struct twine__dict * dict, int num);
static uint32_t locate_bucket(struct twine__dict * dict, uint64_t cookie,
                              struct twine__dict_table ** table);

static uint64_t siphash(uint64_t seed[2], uint64_t cookie);


/* Initialize a dict instance. The call returns zero or success, or
 * TWINE_ENOMEM if a necessary allocation failed. */
int twine__dict_init(struct twine__dict * dict, struct twine_conf * conf, uint64_t seed[2]) {
    struct twine_conn ** entries;
    int i;

    /* Allocate the initial array of hash buckets. */
    entries = conf->malloc(MIN_TABLE_SIZE * sizeof(*entries));
    if (entries == NULL)
        return TWINE_ENOMEM;

    for (i = 0; i < MIN_TABLE_SIZE; i++)
        entries[i] = NULL;

    /* Initialize the dict instance. */
    dict->tables[0] = (struct twine__dict_table) {
        .entries = entries,
        .size = MIN_TABLE_SIZE,
        .mask = MIN_TABLE_SIZE - 1,
    };

    dict->split = 0;
    dict->seed[0] = seed[0];
    dict->seed[1] = seed[1];
    dict->count = 0;
    dict->conf = conf;

    return TWINE_OK;
}


/* Free the dict's internal hash table(s). */
void twine__dict_destroy(struct twine__dict * dict) {
    dict->conf->free(dict->tables[0].entries);

    /* If we've created a second hash table, free its storage too. */
    if (dict->split > 0)
        dict->conf->free(dict->tables[1].entries);
}


/* Look up a connection in the dict by its local connection cookie. The
 * returned pointer will be NULL if no matching entry could be found. */
struct twine_conn * twine__dict_find(struct twine__dict * dict, uint64_t cookie) {
    struct twine__dict_table * table;
    struct twine_conn * conn;
    uint32_t index;

    /* If we're resizing the underlying hash table, move some buckets. */
    if (dict->split > 0)
        migrate_buckets(dict, 1);

    /* Search through all chained entries in the bucket. */
    index = locate_bucket(dict, cookie, &table);
    conn = table->entries[index];

    while (conn != NULL && conn->local_cookie != cookie)
        conn = conn->chain;

    return conn;
}


/* Add a connection entry to the dict. The call will return zero on success,
 * or TWINE_ENOMEM if the dict is large enough that the underlying hash table
 * should be resized but the allocation failed.
 *
 * The implementation makes the assumption that local connection cookies are
 * unique, and that the same connection won't be inserted twice. */
int twine__dict_add(struct twine__dict * dict, struct twine_conn * conn) {
    struct twine__dict_table * table;
    struct twine_conn * head;
    uint32_t index;
    int ret;

    /* If we're resizing the underlying hash table, move some buckets.
     * Otherwise, see if the underlying hash table needs resizing. */
    if (dict->split > 0) {
        migrate_buckets(dict, 4);
    } else {
        ret = maybe_resize(dict, 1);
        if (ret != 0)
            return ret;
    }

    /* Find the relevant hash table bucket. */
    index = locate_bucket(dict, conn->local_cookie, &table);

    /* Insert the connection at the head of the bucket. */
    head = table->entries[index];
    conn->chain = head;
    table->entries[index] = conn;

    /* Update the entry count. */
    dict->count++;

    return TWINE_OK;
}


/* Remove a connection entry from the dict. */
int twine__dict_remove(struct twine__dict * dict, uint64_t cookie) {
    struct twine__dict_table * table;
    struct twine_conn * conn, ** prev;
    uint32_t index;
    int ret;

    /* If we're resizing the underlying hash table, move some buckets.
     * Otherwise, see if the underlying hash table needs resizing. */
    if (dict->split > 0) {
        migrate_buckets(dict, 4);
    } else {
        ret = maybe_resize(dict, -1);
        if (ret != 0)
            return ret;
    }

    /* Find the head of the relevant hash bucket. */
    index = locate_bucket(dict, cookie, &table);
    prev = &table->entries[index];

    while ((conn = *prev) != NULL) {
        /* If we find a match, unlink it and stop. */
        if (conn->local_cookie == cookie) {
            *prev = conn->chain;
            conn->chain = NULL;
            dict->count--;
            break;
        }

        /* We can only unlink the connection if we keep track of the
         * previous pointer. */
        prev = &conn->chain;
    }

    return TWINE_OK;
}


/* Shrink or grow the dict's underlying hash table if utilization is too high
 * or too low. */
static int maybe_resize(struct twine__dict * dict, int delta) {
    struct twine_conn ** entries;
    uint64_t count;
    uint32_t size;
    uint32_t i;

    /* Make access to these variables slightly more ergonomic. */
    count = dict->count;
    size = dict->tables[0].size;

    /* Resize the hash table if it now contains as many entries as there are
     * buckets, or if at least 75% of the buckets are empty. */
    if (delta > 0 && size < MAX_TABLE_SIZE && count >= (uint64_t) size)
        size <<= 1;
    else if (delta < 0 && size > MIN_TABLE_SIZE && count <= (uint64_t) (size/4))
        size >>= 1;
    else
        return TWINE_OK;

    /* Allocate the underlying storage for our new hash table. */
    entries = dict->conf->malloc(size * sizeof(*entries));
    if (entries == NULL)
        return TWINE_ENOMEM;

    for (i = 0; i < size; i++)
        entries[i] = NULL;

    /* Initialize the spare table. */
    dict->tables[1] = (struct twine__dict_table) {
        .entries = entries,
        .size = size,
        .mask = size - 1,
    };

    /* Migrate the first bucket in the old hash table, solely so we can set
     * the `split` field to a non-zero value. */
    migrate_bucket(dict, 0);
    dict->split = 1;

    return TWINE_OK;
}


/* Move all entries from a bucket in the current hash table to their new
 * positions in the new hash table. */
static void migrate_bucket(struct twine__dict * dict, uint32_t index) {
    struct twine_conn * conn, * next;
    uint32_t key;

    /* Grab the first entry, then clear the bucket. */
    conn = dict->tables[0].entries[index];
    dict->tables[0].entries[index] = NULL;

    /* Move all entries in the original chain. */
    while (conn != NULL) {
        next = conn->chain;

        /* Move the connection struct to its new home bucket. */
        key = (uint32_t) siphash(dict->seed, conn->local_cookie);
        index = key & dict->tables[1].mask;

        conn->chain = dict->tables[1].entries[index];
        dict->tables[1].entries[index] = conn;

        /* Move on to the next chained entry. */
        conn = next;
    }
}


/* Move `num` buckets from the current hash table to the next. This function
 * allows us to amortize the cost of resizing hash tables over many read or
 * write operations. */
static void migrate_buckets(struct twine__dict * dict, int num) {
    int i;

    /* Move up to `num` buckets to the resized hash table. */
    for (i = 0; i < num && dict->split > 0; i++) {
        migrate_bucket(dict, dict->split);

        /* Update the split index. Because the hash table size is always a
         * power of two, the mask operation makes the increment wrap to zero
         * when we're done. */
        dict->split = (dict->split + 1) & dict->tables[0].mask;
    }

    /* Once all bucket entries have been moved, drop the old hash table. */
    if (dict->split == 0) {
        dict->conf->free(dict->tables[0].entries);
        dict->tables[0] = dict->tables[1];
    }
}


/* Determine which hash table bucket a particular connection cookie maps to.
 * The hash table's address will be stored in the `table` pointer. */
static uint32_t locate_bucket(struct twine__dict * dict, uint64_t cookie,
                              struct twine__dict_table ** table) {
    uint32_t key, index;

    /* Find the relevant bucket in the main hash table. */
    key = (uint32_t) siphash(dict->seed, cookie);
    index = key & dict->tables[0].mask;

    /* If we're in the process of moving to a new hash table, and our bucket
     * has already been moved, look for the new bucket instead. */
    if (index < dict->split) {
        index = key & dict->tables[1].mask;
        *table = &dict->tables[1];
    } else {
        *table = &dict->tables[0];
    }

    return index;
}


/* SipHash helper macros. */
#define ROTL64(x, n)                                                           \
    ((uint64_t) (((x) << (n)) | ((x) >> (64 - (n)))))

#define SIPHASH_ROUND(v0, v1, v2, v3)                                          \
    do {                                                                       \
        v0 += v1;  v1 = ROTL64(v1, 13);  v1 ^= v0;  v0 = ROTL64(v0, 32);       \
        v2 += v3;  v3 = ROTL64(v3, 16);  v3 ^= v2;                             \
        v0 += v3;  v3 = ROTL64(v3, 21);  v3 ^= v0;                             \
        v2 += v1;  v1 = ROTL64(v1, 17);  v1 ^= v2;  v2 = ROTL64(v2, 32);       \
    } while (0)


/* Unrolled SipHash-2-4 implementation operating on only 64 bits of input. */
static uint64_t siphash(uint64_t seed[2], uint64_t cookie) {
    uint64_t v0, v1, v2, v3;

    /* Initialize state. */
    v0 = 0x736f6d6570736575ULL ^ seed[0];
    v1 = 0x646f72616e646f6dULL ^ seed[1];
    v2 = 0x6c7967656e657261ULL ^ seed[0];
    v3 = 0x7465646279746573ULL ^ seed[1];

    /* Mix in the single 8-byte block of input. */
    v3 ^= cookie;
    SIPHASH_ROUND(v0, v1, v2, v3);
    SIPHASH_ROUND(v0, v1, v2, v3);
    v0 ^= cookie;

    /* Mix in the input length. */
    v3 ^= (uint64_t) 8 << 56;
    SIPHASH_ROUND(v0, v1, v2, v3);
    SIPHASH_ROUND(v0, v1, v2, v3);
    v0 ^= (uint64_t) 8 << 56;

    /* Finalize the hash. */
    v2 ^= 0xff;
    SIPHASH_ROUND(v0, v1, v2, v3);
    SIPHASH_ROUND(v0, v1, v2, v3);
    SIPHASH_ROUND(v0, v1, v2, v3);
    SIPHASH_ROUND(v0, v1, v2, v3);

    return (v0 ^ v1 ^ v2 ^ v3);
}
