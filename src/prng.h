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

#ifndef LIBTWIST_PRNG_H
#define LIBTWIST_PRNG_H

#include <nectar.h>

#include "include/twist.h"
#include "src/env.h"


/* Generates non-deterministic bits using ChaCha20 keystreams. */
struct twist__prng {
    /* ChaCha20 context. */
    struct nectar_chacha20_ctx cx;

    /* Buffer of psuedo-random bytes, which lets us generate larger batches
     * of non-deterministic bits at a time. */
    uint8_t * buf;

    /* Total size of the `buf` array, and a count of how many bytes have
     * already been consumed from it. When `consumed` == `size`, it's time to
     * refill the buffer. */
    size_t size;
    size_t consumed;

    /* This counter dictates how many more times we are allowed to fill the
     * internal buffer using the keystream generated by the same ChaCha20
     * context (key). */
    unsigned int reseed;

    /* Environment. */
    struct twist__env * env;
};


/* Initialize the PRNG context. Returns TWIST_ENOMEM if a necessary memory
 * allocation fails, otherwise TWIST_OK. */
int twist__prng_init(struct twist__prng * prng, struct twist__env * env);

/* Free the PRNG context's allocated memory. */
void twist__prng_clear(struct twist__prng * prng);

/* Read `len` non-deterministic bytes into `buf`. If the PRNG's internal
 * ChaCha20 context needs to be rekeyed, and user-supplied `read_entropy`
 * function fails, the call will fail with TWIST_EENTPY. */
int twist__prng_read(struct twist__prng * prng, uint8_t * buf, size_t len);


#endif
