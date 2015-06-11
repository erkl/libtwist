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

#ifndef LIBTWINE_ENV_H
#define LIBTWINE_ENV_H

#include "include/twine.h"
#include "src/addr.h"


/* The functions defined in the `twine__env` struct form the sole interface
 * used by sockets to interact with the outside world. */
struct twine__env {
    /* User-provided function for sending UDP packets. */
    void (*send_packet)(const struct sockaddr * addr, socklen_t addrlen,
                        const uint8_t * payload, size_t len, void * priv);

    /* User-provided function which can be used to read truly random bytes,
     * preferably using a source like `/dev/urandom`. */
    size_t (*read_entropy)(uint8_t * buf, size_t len, void * priv);

    /* Arbitrary pointer provided by the user for the socket, which will be
     * passed along with each call to any of the functions above. */
    void * priv;
};


/* Send a UDP packet to `addr`. */
static inline void twine__env_send(struct twine__env * env, const struct twine__addr * addr,
                                   const uint8_t * payload, size_t len) {
    env->send_packet((const struct sockaddr *) addr, (socklen_t) addr->len,
                     payload, len, env->priv);
}


/* Read random `len` bytes of random data. */
static inline size_t twine__env_entropy(struct twine__env * env, uint8_t * buf, size_t len) {
    return env->read_entropy(buf, len, env->priv);
}


#endif
