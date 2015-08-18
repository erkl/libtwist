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

    /* When is the next time-based event scheduled to occur?
     * NOTE: Managed in conn.c, but used by sock.c and heap.c. */
    int64_t next_tick;

    /* Local and remote connection cookies. */
    uint64_t local_cookie;
    uint64_t remote_cookie;

    /* Buffers for outgoing and incoming data. */
    struct twist__buffer write_buffer;
    struct twist__buffer read_buffer;

    /* Current position in the socket's min-heap.
     * NOTE: Managed in heap.c. */
    uint32_t heap_index;

    /* Intrusive pointer for hash table chaining.
     * NOTE: Managed in dict.c. */
    struct twist__conn * chain;

    /* Intrusive pointers for storing the connection in its socket's linked
     * list of pending accepted connections.
     * NOTE: Managed in sock.c. */
    struct twist__conn * prev;
    struct twist__conn * next;
};


/* Allocate and initialize a connection struct. */
int twist__conn_create(struct twist__conn ** connptr, struct twist__sock * sock, uint64_t local_cookie);

/* Free all memory owned by the connection struct. */
void twist__conn_destroy(struct twist__conn ** connptr);


/* Begin the process of establishing a connection to a remote host with a
 * newly created `twist__conn` struct. */
int twist__conn_dial(struct twist__conn * conn, const struct sockaddr * addr,
                     socklen_t addrlen, int64_t now);

/* Begin the process of accepting an incoming connection request with a newly
 * created `twist__conn` struct. */
int twist__conn_accept(struct twist__conn * conn,
                       uint64_t remote_cookie, const uint8_t pk[64],
                       const struct sockaddr * addr, socklen_t addrlen, int64_t now);


/* Propagate a time event to the connection's state machine. */
int twist__conn_tick(struct twist__conn * conn, int64_t now);

/* Feed a received packet to the connection's state machine. */
int twist__conn_recv(struct twist__conn * conn, char type,
                     struct twist__packet * packet, int64_t now);


#endif
