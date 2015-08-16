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

    /* Singly-linked list of packets that need to be kept around for a bit
     * because of our guarantee that everything passed to a `send_packet`
     * function will be valid until the next operation on the socket. */
    struct twist__packet * lingering;

    /* Connections ordered by their `next_tick` values. */
    struct twist__heap heap;

    /* Hash table of connections keyed by their local cookies. */
    struct twist__dict dict;

    /* Circular linked list of accepted connections. */
    struct twist__conn * accepted;

    /* Key used when encrypting and signing handshake tickets. */
    uint8_t ticket_key[32];

    /* Strike-register for handshake tickets. */
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


/* Add a connection to the socket's internal data structures. */
int twist__sock_add(struct twist__sock * sock, struct twist__conn * conn);

/* Remove a connection from the socket's internal data structures. */
void twist__sock_remove(struct twist__sock * sock, struct twist__conn * conn);


/* Feed a clock tick to the socket. */
int twist__sock_tick(struct twist__sock * sock, int64_t now);

/* Feed an incoming packet to the socket. */
int twist__sock_recv(struct twist__sock * sock,
                     const struct sockaddr * addr, socklen_t addrlen,
                     const uint8_t * payload, size_t len, int64_t now);



#endif
