/* Copyright (c) 2015, Erik Lundin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the copyright holder nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

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
