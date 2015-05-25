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

#ifndef LIBTWINE_SOCK_H
#define LIBTWINE_SOCK_H

#include "include/twine.h"
#include "src/dict.h"
#include "src/env.h"
#include "src/prng.h"


/* Socket handle, opaque to the user. */
struct twine_sock {
    /* Hash table containing all known connections, keyed by their
     * connection cookies. */
    struct twine__dict dict;

    /* Circular linked list of established connections. */
    struct twine_conn * conns;

    /* Socket-wide psuedo-random number generator. */
    struct twine__prng prng;

    /* Environment. */
    struct twine__env env;
};


#endif
