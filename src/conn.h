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

#ifndef LIBTWINE_CONN_H
#define LIBTWINE_CONN_H

#include "include/twine.h"
#include "src/buffer.h"
#include "src/packet.h"


/* Connection handle, opaque to the user. */
struct twine_conn {
    /* Connection state. */
    int state;

    /* Owning socket. */
    struct twine_sock * sock;

    /* Local and remote connection cookies. */
    uint64_t local_cookie;
    uint64_t remote_cookie;

    /* When is the next time-based event scheduled to occur? */
    int64_t next_tick;

    /* Current position in the socket's min-heap. */
    uint32_t heap_index;

    /* Intrusive pointer for hash table chaining. */
    struct twine_conn * chain;

    /* Buffers for outgoing and incoming data. */
    struct twine__buffer write_buffer;
    struct twine__buffer read_buffer;
};


/* Allocate and initialize a connection struct. */
int64_t twine__conn_create(struct twine_conn ** connptr, struct twine_sock * sock,
                           uint64_t local_cookie, uint64_t remote_cookie);

/* Free all memory owned by the connection struct. */
void twine__conn_destroy(struct twine_conn ** connptr);


/* Propagate a time event to the connection's state machine. */
int64_t twine__conn_tick(struct twine_conn * conn, int64_t now);

/* Feed a received packet to the connection's state machine. */
int64_t twine__conn_recv(struct twine_conn * conn, struct twine__packet * packet, int64_t now);


#endif
