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

#ifndef LIBTWIST_ADDR_H
#define LIBTWIST_ADDR_H

#include "include/twist.h"


/* Maximum number of address bytes that will fit in a `twist__addr`. */
#define MAX_ADDR_LEN  30


/* This struct represents a network address. It basically functions as a much
 * smaller `struct sockaddr_storage`, with a baked-in socklen_t. */
struct twist__addr {
    /* 30 bytes of 8-byte aligned storage. */
    uint64_t storage;
    uint8_t pad[22];

    /* Address size. */
    uint16_t len;
};


/* Construct an address from a plain `sockaddr` struct. */
void twist__addr_load(struct twist__addr * addr,
                      const struct sockaddr * sockaddr, socklen_t socklen);

/* Copy the value of the address `from` into `addr`. */
void twist__addr_copy(struct twist__addr * addr, const struct twist__addr * from);


#endif
