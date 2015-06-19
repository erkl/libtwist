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

#ifndef LIBTWINE_SOCK_H
#define LIBTWINE_SOCK_H

#include "include/twine.h"
#include "src/dict.h"
#include "src/env.h"
#include "src/heap.h"
#include "src/pool.h"
#include "src/prng.h"
#include "src/register.h"


/* Socket state. */
struct twine_sock {
    /* The beginning of time, as far as this socket is concerned. */
    int64_t first_tick;

    /* We make sure that time never goes backwards by always keeping track
     * of the previous tick value. */
    int64_t last_tick;

    /* This field holds the next clock tick which will affect a connection's
     * state. Essentially a shortcut for `twine__heap_peek(heap)->next_tick`. */
    int64_t next_tick;

    /* Connections ordered by their `next_tick` values. */
    struct twine__heap heap;

    /* Hash table of connections keyed by their local cookies. */
    struct twine__dict dict;

    /* Strike-register for source address tokens. */
    struct twine__register reg;

    /* Shared memory pool. */
    struct twine__pool pool;

    /* Socket-wide psuedo-random number generator. */
    struct twine__prng prng;

    /* Environment. */
    struct twine__env env;
};


#endif
