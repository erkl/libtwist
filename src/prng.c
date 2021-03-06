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

#include "src/mem.h"
#include "src/prng.h"


/* Constants defining a) the size of each PRNG instance's internal buffer,
 * and b) how many times we may fill it before re-keying our ChaCha20 context.
 *
 * TODO: Would letting the user specify these values be a decent idea,
 *       or a terrible one? */
#define BUFFER_SIZE      1024
#define RESEED_INTERVAL  64


/* Static functions. */
static int seed(struct twist__prng * prng);


/* Initialize the PRNG context. Returns TWIST_ENOMEM if a necessary memory
 * allocation fails, otherwise TWIST_OK. */
int twist__prng_init(struct twist__prng * prng, struct twist__env * env) {
    uint8_t * buf;

    /* Allocate the buffer. */
    buf = twist__malloc(BUFFER_SIZE);
    if (buf == NULL)
        return TWIST_ENOMEM;

    /* Initialize fields. */
    prng->buf = buf;
    prng->size = BUFFER_SIZE;
    prng->consumed = BUFFER_SIZE;
    prng->reseed = 0;
    prng->env = env;

    return TWIST_OK;
}


/* Free the PRNG context's allocated memory. */
void twist__prng_clear(struct twist__prng * prng) {
    twist__free(prng->buf);
}


/* Read `len` non-deterministic bytes into `buf`. If the PRNG's internal
 * ChaCha20 context needs to be rekeyed, and user-supplied `read_entropy`
 * function fails, the call will fail with TWIST_EENTPY. */
int twist__prng_read(struct twist__prng * prng, uint8_t * buf, size_t len) {
    size_t n;
    int ret;

    while (len > 0) {
        /* Have we consumed the whole buffer? */
        if (prng->consumed == prng->size) {
            /* Is it time to rekey the ChaCha20 context? */
            if (prng->reseed == 0) {
                ret = seed(prng);
                if (ret != TWIST_OK)
                    return ret;
            }

            /* Generate the next BUFFER_SIZE bytes of keystream from the
             * current ChaCha20 context. */
            memset(prng->buf, 0, BUFFER_SIZE);
            nectar_chacha20_xor(&prng->cx, prng->buf, prng->buf, BUFFER_SIZE);

            /* Update the reseed counter. */
            prng->reseed--;
        }

        /* Copy `n` bytes of output into `buf`. */
        n = prng->size - prng->consumed;
        if (n > len)
            n = len;

        memcpy(buf, prng->buf + prng->consumed, n);
        prng->consumed += n;

        /* Advance. */
        buf += n;
        len -= n;
    }

    return TWIST_OK;
}


/* Seed a new keystream to use for random number generation. */
static int seed(struct twist__prng * prng) {
    uint8_t buf[40];
    int ret;

    /* Request a 40-byte seed. */
    ret = twist__env_entropy(prng->env, buf, 40);
    if (ret != TWIST_OK)
        return ret;

    /* Set up the ChaCha20 context. */
    nectar_chacha20_init(&prng->cx, buf, buf + 32);
    prng->reseed = RESEED_INTERVAL;

    return TWIST_OK;
}
