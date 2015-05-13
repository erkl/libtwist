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

#include <string.h>

#include "include/twine.h"
#include "src/buffer.h"


/* Maximum capacity of a slab. */
#define BUFFER_SLAB_SIZE                                                       \
    (POOL_OBJECT_SIZE - sizeof(struct twine__buffer_slab))


/* Calculate the amount of trailing unused space in a slab. */
#define UNUSED(s)                                                              \
    ((size_t) (((uint8_t *) (s)) + POOL_OBJECT_SIZE - (s)->end))


/* Static functions. */
struct twine__buffer_slab * alloc(struct twine__buffer * bufr, size_t len);
size_t append(struct twine__buffer_slab * slab, const uint8_t * buf, size_t len);


/* Initialize the buffer's internal fields. */
void twine__buffer_init(struct twine__buffer * bufr, struct twine__pool * pool) {
    bufr->head = NULL;
    bufr->tail = NULL;
    bufr->size = 0;
    bufr->pool = pool;
}


/* Discard all data and return the buffer's slabs to the object pool. */
void twine__buffer_clear(struct twine__buffer * bufr) {
    struct twine__buffer_slab * first, * curr, * next;

    /* If the buffer is already empty, do nothing. */
    if (bufr->head == NULL)
        return;

    /* Free one slab at a time. */
    first = bufr->head;
    curr = first;

    do {
        next = curr->next;
        twine__pool_free(bufr->pool, curr);
        curr = next;
    } while (curr != first);

    /* Reset internal fields. */
    bufr->head = NULL;
    bufr->size = 0;
}


/* Write a chunk of data to the buffer. Unless a necessary memory allocation
 * fails (in which case no data is written and TWINE_ENOMEM is returned), the
 * full write is guaranteed to complete successfully. The `len` argument must
 * not be greater than SSIZE_MAX. */
ssize_t twine__buffer_write(struct twine__buffer * bufr, const uint8_t * buf, size_t len) {
    struct twine__buffer_slab * added;
    size_t cap, rem, n;

    /* Bail early with no input. */
    if (len == 0)
        return 0;

    /* If there is already some free space in the last slab, take that
     * into account. */
    cap = (bufr->tail != NULL ? UNUSED(bufr->tail) : 0);

    /* Allocate one or more additional slabs if we don't have enough room. */
    if (cap < len) {
        added = alloc(bufr, len - cap);
        if (added == NULL)
            return TWINE_ENOMEM;

        if (bufr->head == NULL) {
            bufr->head = added;
            bufr->tail = added;
        } else {
            bufr->tail->next = added;
        }
    }

    /* Write the user-provided input data into slabs. We use `bufr->tail` as
     * a cursor when walking through the list. */
    rem = len;

    for (;;) {
        n = append(bufr->tail, buf, rem);

        buf += n;
        rem -= n;

        /* Stop when we're done. Obviously. */
        if (rem == 0)
            break;

        /* If we didn't just break out of the loop, it means we're still not
         * done. This in turn means that there are more slabs for us to use. */
        bufr->tail = bufr->tail->next;
    }

    /* And we're done. Finally. */
    bufr->size += len;

    return len;
}


/* Read data from the buffer. This call can't fail, only return 0 when the
 * buffer is empty. The `len` argument must not be greater than SSIZE_MAX. */
ssize_t twine__buffer_read(struct twine__buffer * bufr, uint8_t * buf, size_t len) {
    struct twine__buffer_slab * slab;
    ssize_t nread, n;

    /* Initialize the counter. */
    nread = 0;

    /* Keep reading until we're either out of buffered data, or out of room
     * for the output. */
    while (len > 0 && bufr->size > 0) {
        slab = bufr->head;

        /* Be careful not to try to read too much. */
        n = (size_t) (slab->end - slab->start);
        if (n > len)
            n = len;

        memcpy(buf, slab->start, n);
        slab->start += n;

        /* Discard the slab if we've now emptied it. */
        if (slab->start == slab->end) {
            if (slab->next != NULL) {
                bufr->head = slab->next;
            } else {
                bufr->head = NULL;
                bufr->tail = NULL;
            }

            twine__pool_free(bufr->pool, slab);
        }

        /* Update counters. */
        buf += n;
        len -= n;

        bufr->size -= n;
        nread += n;
    }

    return nread;
}


/* Get the number of bytes of data currently stored in the buffer. */
size_t twine__buffer_size(struct twine__buffer * bufr) {
    return bufr->size;
}


/* Allocate enough slabs to fit `cap` bytes of data. Returns the first item of
 * a singly linked list of slabs on success, or NULL if an allocation failed. */
struct twine__buffer_slab * alloc(struct twine__buffer * bufr, size_t cap) {
    struct twine__buffer_slab * head, * next;

    /* Request one slab from the pool at a time. */
    for (;;) {
        next = twine__pool_alloc(bufr->pool);
        if (next == NULL)
            goto err;

        /* Initialize the empty slab and prepend it to the linked list. */
        next->start = ((uint8_t *) next) + sizeof(struct twine__buffer_slab);
        next->end = next->start;

        next->next = head;
        head = next;

        /* Stop if this was the last slab we needed. */
        if (cap <= BUFFER_SLAB_SIZE)
            break;

        cap -= BUFFER_SLAB_SIZE;
    }

    return head;

    /* If we get here, an allocation failed. This means that `head` is the
     * head of a list of 0 or more slabs that we allocated but won't use. */
err:
    while (head != NULL) {
        next = head->next;
        twine__pool_free(bufr->pool, head);
        head = next;
    }

    return NULL;
}


/* Append up to `len` bytes of data to a slab. */
size_t append(struct twine__buffer_slab * slab, const uint8_t * buf, size_t len) {
    size_t n = UNUSED(slab);
    if (n > len)
        n = len;

    memcpy(slab->end, buf, n);
    slab->end += n;

    return n;
}
