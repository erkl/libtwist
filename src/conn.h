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

#ifndef LIBTWIST_CONN_H
#define LIBTWIST_CONN_H

#include "include/twist.h"
#include "src/buffer.h"
#include "src/packet.h"


/* Connection state. */
struct twist__conn {
    /* Owning socket. */
    struct twist__sock * sock;

    /* Connection state. */
    int state;

    /* Local and remote connection cookies. */
    uint64_t local_cookie;
    uint64_t remote_cookie;

    /* Buffers for outgoing and incoming data. */
    struct twist__buffer write_buffer;
    struct twist__buffer read_buffer;

    /* When is the next time-based event scheduled to occur?
     * NOTE: Managed by sock.c, used by heap.c. */
    int64_t next_tick;

    /* Current position in the socket's min-heap.
     * NOTE: Managed by heap.c. */
    uint32_t heap_index;

    /* Intrusive pointer for hash table chaining.
     * NOTE: Managed by dict.c. */
    struct twist__conn * chain;
};


/* Allocate and initialize a connection struct. */
int64_t twist__conn_create(struct twist__conn ** connptr, struct twist_sock * sock,
                           uint64_t local_cookie, uint64_t remote_cookie);

/* Free all memory owned by the connection struct. */
void twist__conn_destroy(struct twist__conn ** connptr);


/* Propagate a time event to the connection's state machine. */
int64_t twist__conn_tick(struct twist__conn * conn, int64_t now);

/* Feed a received packet to the connection's state machine. */
int64_t twist__conn_recv(struct twist__conn * conn, struct twist__packet * packet, int64_t now);


#endif
