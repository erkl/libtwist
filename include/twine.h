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


/* TODO: Documentation. */
int twine_create(struct twine_sock ** sockptr);

/* TODO: Documentation. */
int64_t twine_tick(struct twine_sock * sock, int64_t now);

/* TODO: Documentation. */
int64_t twine_recv(struct twine_sock * sock,
                   const struct sockaddr * addr, socklen_t addrlen,
                   const uint8_t * buf, size_t len, int64_t now);

/* TODO: Documentation. */
int twine_destroy(struct twine_sock ** sockptr);


/* TODO: Documentation. */
int twine_dial(struct twine_sock * sock, struct twine_conn ** connptr,
               const struct sockaddr * addr, socklen_t addrlen, int64_t timeout);

/* TODO: Documentation. */
int twine_accept(struct twine_sock * sock, struct twine_conn ** connptr, int64_t timeout);


/* TODO: Documentation. */
ssize_t twine_read(struct twine_conn * conn, uint8_t * buf, size_t len);

/* TODO: Documentation. */
ssize_t twine_write(struct twine_conn * conn, const uint8_t * buf, size_t len);

/* TODO: Documentation. */
int twine_flush(struct twine_conn * conn);

/* TODO: Documentation. */
int twine_close(struct twine_conn * conn);

/* TODO: Documentation. */
int twine_drop(struct twine_conn ** connptr);


#endif
