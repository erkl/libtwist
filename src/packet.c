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

#include <string.h>

#include "include/twist.h"
#include "src/packet.h"


/* Initialize a packet. */
void twist__packet_init(struct twist__packet * pkt,
                        const struct sockaddr * addr, socklen_t addrlen,
                        const uint8_t * payload, size_t len) {
    uint8_t * base;

    /* Use the space after the `struct twist__packet` fields to store the
     * packet's payload. This relies on the assumption that `pkt` is an object
     * managed by a `twist__pool`.
     *
     * The incantations below calculate the location of the first byte
     * immediately after the packet struct itself, then rounds that address
     * up to the nearest multiple of 8. */
    base = (uint8_t *) (((uintptr_t) (pkt + 1) + 7) & ~((uintptr_t) 7));

    /* Store the address. */
    twist__addr_load(&pkt->addr, addr, addrlen);

    /* Copy the payload. */
    memcpy(base, payload, len);

    pkt->payload = base;
    pkt->len = len;

    /* Initialize the list pointer. */
    pkt->next = NULL;
}
