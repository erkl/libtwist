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

#ifndef LIBTWINE_REGISTER_H
#define LIBTWINE_REGISTER_H

#include "include/twine.h"


/* The `twine__register` struct generates and validates single-use, fixed
 * lifetime "tokens", used by sockets when verifying the remote address of a
 * connecting party. It does this rather efficiently (1 bit per token, plus
 * a small constant overhead) using a circular bitset.
 *
 * Because the generated tokens will be encrypted and signed by the socket
 * before being sent, the current register implementation doesn't verify that
 * the tokens being passed to `twine__register_claim` were in fact generated
 * by `twine__register_reserve`. */
struct twine__register {
    /* Circular array storing the starting offsets (in the `bits` array) for
     * the last `lifetime` buckets (a bucket being second-long intervals that
     * tokens are grouped into). */
    uint32_t * offsets;

    /* Lifetime in seconds of the tokens generated by this register.
     * Also the size of the `offsets` array. */
    uint32_t lifetime;

    /* Stores the last bucket we reserved a token in. */
    uint32_t cursor;

    /* Number of tokens that have been created in the current bucket. */
    uint32_t counter;

    /* Circular bitset storage, in blocks of 32 bits. */
    uint32_t * bits;

    /* Size of the `bits` array (which is always a power of 2), and a simple
     * mask to replace `x % size` with `x & mask`. */
    uint32_t size;
    uint32_t mask;
};


/* Initialize the register. Returns TWINE_ENOMEM if a necessary allocation
 * failed, otherwise TWINE_OK. */
int twine__register_init(struct twine__register * reg, uint32_t lifetime, int64_t now);

/* Free all heap memory managed by the register. */
void twine__register_destroy(struct twine__register * reg);


/* Generate a new token. Returns TWINE_ENOMEM if the underlying storage array
 * is full and an attempt to grow it failed, or TWINE_EAGAIN if we've already
 * reached the hard limit of tokens generated per second (2,147,483,647,
 * or 2^31 - 1); otherwise TWINE_OK. */
int twine__register_reserve(struct twine__register * reg, uint32_t token[2], int64_t now);

/* Claim a token, removing it from the register. Returns TWINE_EINVAL if the
 * token has expired or has already been claimed; otherwise TWINE_OK. */
int twine__register_claim(struct twine__register * reg, const uint32_t token[2], int64_t now);

/* Remove any expired tokens from the register, then shrink its internal storage
 * array if possible. Returns TWINE_ENOMEM if a necessary allocation failed,
 * otherwise TWINE_OK. */
int twine__register_reduce(struct twine__register * reg, int64_t now);


#endif
