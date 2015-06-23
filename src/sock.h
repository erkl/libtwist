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

#ifndef LIBTWIST_SOCK_H
#define LIBTWIST_SOCK_H

#include "include/twist.h"
#include "src/dict.h"
#include "src/env.h"
#include "src/heap.h"
#include "src/pool.h"
#include "src/prng.h"
#include "src/register.h"


/* Socket state. */
struct twist__sock {
    /* We make sure that time never goes backwards by always keeping track
     * of the previous tick value. */
    int64_t last_tick;

    /* This field holds the next clock tick which will affect a connection's
     * state. Essentially a shortcut for `twist__heap_peek(heap)->next_tick`. */
    int64_t next_tick;

    /* Connections ordered by their `next_tick` values. */
    struct twist__heap heap;

    /* Hash table of connections keyed by their local cookies. */
    struct twist__dict dict;

    /* Strike-register for source address tokens. */
    struct twist__register reg;

    /* Shared memory pool. */
    struct twist__pool pool;

    /* Socket-wide psuedo-random number generator. */
    struct twist__prng prng;

    /* Environment. */
    struct twist__env env;
};


/* Allocate and initialize a new socket. */
int twist__sock_create(struct twist__sock ** sockptr, struct twist__env * env);


/* Free a socket. Fails with TWIST_EAGAIN if the socket in question has any
 * open (as in not yet dropped) connections. */
int twist__sock_destroy(struct twist__sock ** sockptr);


/* Feed a clock tick to the socket. */
int64_t twist__sock_tick(struct twist__sock * sock, int64_t now);


#endif
