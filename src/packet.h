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

#ifndef LIBTWIST_PACKET_H
#define LIBTWIST_PACKET_H

#include "include/twist.h"
#include "src/addr.h"


/* Control packet sizes. */
#define HANDSHAKE_PACKET_SIZE  176
#define TICKET_PACKET_SIZE     168


/* Describes an incoming or outgoing packet. */
struct twist__packet {
    /* Source/destination address. */
    struct twist__addr addr;

    /* Packet payload. The payload will always be allocated immediately after
     * the packet struct header itself, but an explicit pointer is way better
     * than pointer arithmetic. */
    uint8_t * payload;

    /* Packet payload size. */
    size_t len;

    /* Next packet in a linked list. */
    struct twist__packet * next;
};


/* Initialize a packet. */
void twist__packet_init(struct twist__packet * pkt,
                        const struct sockaddr * addr, socklen_t addrlen,
                        const uint8_t * payload, size_t len);


#endif
