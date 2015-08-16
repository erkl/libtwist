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

#include <nectar.h>

#include "src/dict.h"
#include "src/mem.h"


/* Minimum (default) and maximum hash table sizes. The maximum size may seem
 * arbitrary, but it's the highest we can go while being sure multiplication
 * by 8 (size of 64-bit pointers) won't overflow uint32_t. */
#define MIN_TABLE_SIZE  (1 << 6)
#define MAX_TABLE_SIZE  (1 << 28)


/* Static functions. */
static int maybe_resize(struct twist__dict * dict, int delta);
static void migrate_bucket(struct twist__dict * dict, uint32_t index);
static void migrate_buckets(struct twist__dict * dict, int num);
static uint32_t locate_bucket(struct twist__dict * dict, uint64_t cookie,
                              struct twist__dict_table ** table);


/* Initialize a dict instance. The call returns zero or success, or
 * TWIST_ENOMEM if a necessary allocation failed. */
int twist__dict_init(struct twist__dict * dict, uint8_t seed[16]) {
    struct twist__conn ** buckets;
    int i;

    /* Allocate the initial array of hash buckets. */
    buckets = twist__malloc(MIN_TABLE_SIZE * sizeof(struct twist__conn *));
    if (buckets == NULL)
        return TWIST_ENOMEM;

    for (i = 0; i < MIN_TABLE_SIZE; i++)
        buckets[i] = NULL;

    /* Initialize the dict instance. */
    dict->tables[0] = (struct twist__dict_table) {
        .buckets = buckets,
        .size = MIN_TABLE_SIZE,
        .mask = MIN_TABLE_SIZE - 1,
    };

    dict->split = 0;
    dict->count = 0;

    /* Copy the seed. */
    for (i = 0; i < 16; i++)
        dict->seed[i] = seed[i];

    return TWIST_OK;
}


/* Free the dict's internal hash table(s). */
void twist__dict_clear(struct twist__dict * dict) {
    twist__free(dict->tables[0].buckets);

    /* If we've created a second hash table, free its storage too. */
    if (dict->split > 0)
        twist__free(dict->tables[1].buckets);
}


/* Look up a connection in the dict by its local connection cookie. The
 * returned pointer will be NULL if no matching entry could be found. */
struct twist__conn * twist__dict_find(struct twist__dict * dict, uint64_t cookie) {
    struct twist__dict_table * table;
    struct twist__conn * conn;
    uint32_t index;

    /* If we're resizing the underlying hash table, move some buckets. */
    if (dict->split > 0)
        migrate_buckets(dict, 1);

    /* Search through all chained entries in the bucket. */
    index = locate_bucket(dict, cookie, &table);
    conn = table->buckets[index];

    while (conn != NULL && conn->local_cookie != cookie)
        conn = conn->chain;

    return conn;
}


/* Add a connection entry to the dict. The call will return zero on success,
 * or TWIST_ENOMEM if the dict is large enough that the underlying hash table
 * should be resized but the allocation failed.
 *
 * The implementation makes the assumption that local connection cookies are
 * unique, and that the same connection won't be inserted twice. */
int twist__dict_add(struct twist__dict * dict, struct twist__conn * conn) {
    struct twist__dict_table * table;
    struct twist__conn * head;
    uint32_t index;
    int ret;

    /* Find the relevant hash table bucket. */
    index = locate_bucket(dict, conn->local_cookie, &table);

    /* Insert the connection at the head of the bucket. */
    head = table->buckets[index];
    conn->chain = head;
    table->buckets[index] = conn;

    /* Update the entry count. */
    dict->count++;

    /* If we're resizing the underlying hash table, move some buckets.
     * Otherwise, see if the underlying hash table needs resizing. */
    if (dict->split > 0) {
        migrate_buckets(dict, 4);
    } else {
        ret = maybe_resize(dict, 1);
        if (ret != TWIST_OK)
            return ret;
    }

    return TWIST_OK;
}


/* Remove a connection entry from the dict. */
void twist__dict_remove(struct twist__dict * dict, struct twist__conn * conn) {
    struct twist__dict_table * table;
    struct twist__conn ** prev;
    uint64_t cookie;
    uint32_t index;

    /* Extract the cookie. */
    cookie = conn->local_cookie;

    /* Find the head of the relevant hash bucket. */
    index = locate_bucket(dict, cookie, &table);
    prev = &table->buckets[index];

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

    /* If we're resizing the underlying hash table, move some buckets.
     * Otherwise, see if the underlying hash table needs resizing. */
    if (dict->split > 0) {
        migrate_buckets(dict, 4);
    } else {
        /* TODO: While calls to `twist__dict_remove` must always succeed, there
         *       might be a better way to deal with this potential error? */
        maybe_resize(dict, -1);
    }
}


/* Shrink or grow the dict's underlying hash table if utilization is too high
 * or too low. */
static int maybe_resize(struct twist__dict * dict, int delta) {
    struct twist__conn ** buckets;
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
    else if (delta < 0 && size > MIN_TABLE_SIZE && count <= (uint64_t) (size / 4))
        size >>= 1;
    else
        return TWIST_OK;

    /* Allocate the underlying storage for our new hash table. */
    buckets = twist__malloc(size * sizeof(struct twist__conn *));
    if (buckets == NULL)
        return TWIST_ENOMEM;

    for (i = 0; i < size; i++)
        buckets[i] = NULL;

    /* Initialize the spare table. */
    dict->tables[1] = (struct twist__dict_table) {
        .buckets = buckets,
        .size = size,
        .mask = size - 1,
    };

    /* Migrate the first bucket in the old hash table, solely so we can set
     * the `split` field to a non-zero value. */
    migrate_bucket(dict, 0);
    dict->split = 1;

    return TWIST_OK;
}


/* Move all entries from a bucket in the current hash table to their new
 * positions in the new hash table. */
static void migrate_bucket(struct twist__dict * dict, uint32_t index) {
    struct twist__conn * conn, * next;
    uint32_t key;

    /* Grab the first entry, then clear the bucket. */
    conn = dict->tables[0].buckets[index];
    dict->tables[0].buckets[index] = NULL;

    /* Move all entries in the original chain. */
    while (conn != NULL) {
        next = conn->chain;

        /* Move the connection struct to its new home bucket. */
        key = (uint32_t) nectar_siphash(dict->seed, (const uint8_t *) conn->local_cookie, 8);
        index = key & dict->tables[1].mask;

        conn->chain = dict->tables[1].buckets[index];
        dict->tables[1].buckets[index] = conn;

        /* Move on to the next chained entry. */
        conn = next;
    }
}


/* Move `num` buckets from the current hash table to the next. This function
 * allows us to amortize the cost of resizing hash tables over many read or
 * write operations. */
static void migrate_buckets(struct twist__dict * dict, int num) {
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
        twist__free(dict->tables[0].buckets);
        dict->tables[0] = dict->tables[1];
    }
}


/* Determine which hash table bucket a particular connection cookie maps to.
 * The hash table's address will be stored in the `table` pointer. */
static uint32_t locate_bucket(struct twist__dict * dict, uint64_t cookie,
                              struct twist__dict_table ** table) {
    uint32_t key, index;

    /* Find the relevant bucket in the main hash table. */
    key = (uint32_t) nectar_siphash(dict->seed, (const uint8_t *) cookie, 8);
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
