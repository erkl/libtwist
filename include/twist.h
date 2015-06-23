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

#ifndef LIBTWIST_H
#define LIBTWIST_H

#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>


/* Error codes. */
#define TWIST_OK        ( 0)
#define TWIST_EINVAL    (-1)
#define TWIST_ENOMEM    (-2)
#define TWIST_EENTROPY  (-3)
#define TWIST_EAGAIN    (-4)


/* Opaque socket and connection handles. */
struct twist_sock;
struct twist_conn;


/* TODO: Documentation. */
int twist_create(struct twist_sock ** sockptr);

/* TODO: Documentation. */
int64_t twist_tick(struct twist_sock * sock, int64_t now);

/* TODO: Documentation. */
int64_t twist_recv(struct twist_sock * sock,
                   const struct sockaddr * addr, socklen_t addrlen,
                   const uint8_t * buf, size_t len, int64_t now);

/* TODO: Documentation. */
int twist_destroy(struct twist_sock ** sockptr);


/* TODO: Documentation. */
int twist_dial(struct twist_sock * sock, struct twist_conn ** connptr,
               const struct sockaddr * addr, socklen_t addrlen, int64_t timeout);

/* TODO: Documentation. */
int twist_accept(struct twist_sock * sock, struct twist_conn ** connptr, int64_t timeout);


/* TODO: Documentation. */
ssize_t twist_read(struct twist_conn * conn, uint8_t * buf, size_t len);

/* TODO: Documentation. */
ssize_t twist_write(struct twist_conn * conn, const uint8_t * buf, size_t len);

/* TODO: Documentation. */
int twist_flush(struct twist_conn * conn);

/* TODO: Documentation. */
int twist_close(struct twist_conn * conn);

/* TODO: Documentation. */
int twist_drop(struct twist_conn ** connptr);


#endif
