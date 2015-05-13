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

#ifndef LIBTWINE_PACKET_H
#define LIBTWINE_PACKET_H

#include "include/twine.h"
#include "src/addr.h"


/* Minimum and maximum packet sizes. The maximum size may seem arbitrary, but
 * it was calculated by subtracting the per-packet overhead of PPPoE (8 bytes),
 * IPv6 (40 bytes) and UDP (8 bytes) from Ethernet's MTU (1500 bytes). */
#define MIN_PACKET_SIZE  28
#define MAX_PACKET_SIZE  1444


/* Describes an incoming or outgoing packet. */
struct twine__packet {
    /* Packet payload. The buffer array will always be allocated immediately
     * after the packet struct itself, but I prefer an explicit pointer over
     * a fancy macro. */
    uint8_t * payload;
    size_t len;

    /* Remote address. */
    struct twine__addr remote;

    /* Intrusive list pointers. */
    struct twine__packet * prev;
    struct twine__packet * next;
};


#endif
