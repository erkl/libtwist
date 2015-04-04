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

#ifndef LIBTWINE_H
#define LIBTWINE_H

#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>


/* Error codes. */
#define TWINE_OK        ( 0)
#define TWINE_EINVAL    (-1)
#define TWINE_ENOMEM    (-2)
#define TWINE_EENTROPY  (-3)
#define TWINE_EAGAIN    (-4)


/* Opaque socket and connection handles. */
struct twine_sock;
struct twine_conn;


/* Socket-wide configuration parameters. */
struct twine_conf {
    /* Send a UDP packet. */
    void (*send_packet)(const struct sockaddr * addr, socklen_t addrlen,
                        const uint8_t * buf, size_t len);

    /* Source of random data. */
    size_t (*read_entropy)(uint8_t * buf, size_t len);

    /* Memory management. */
    void * (*malloc)(size_t size);
    void (*free)(void * ptr);
};


/* Allocate and initialize a socket. */
int twine_init(struct twine_sock ** sock, struct twine_conf * conf);

/* Free all resources associated with a socket. */
int twine_shutdown(struct twine_sock ** sock);


/* Signal time passing to the socket. */
int64_t twine_tick(struct twine_sock * sock, int64_t now);

/* Feed a received UDP packet to the socket. */
int64_t twine_recv(struct twine_sock * sock,
                   const struct sockaddr * addr, socklen_t addrlen,
                   const uint8_t * buf, size_t len, int64_t now);


/* Establish a connection to a remote host. */
int twine_dial(struct twine_sock * sock, struct twine_conn ** conn,
               const struct sockaddr * addr, socklen_t addrlen, int64_t timeout);

/* Accept an incoming connection request. */
int twine_accept(struct twine_sock * sock, struct twine_conn ** conn, int64_t timeout);


/* Read data from the connection. */
ssize_t twine_read(struct twine_conn * conn, uint8_t * buf, size_t len);

/* Write data to the connection. */
ssize_t twine_write(struct twine_conn * conn, const uint8_t * buf, size_t len);

/* Flush any data still in the write buffer. */
int twine_flush(struct twine_conn * conn);


/* Get the connection's current state. */
int twine_state(struct twine_conn * conn);


/* Initiate the graceful shutdown of a connection. */
int twine_close(struct twine_conn * conn);

/* Destroy a connection, freeing all resources associated with it. */
int twine_destroy(struct twine_conn ** conn);


#endif
