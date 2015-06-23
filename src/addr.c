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
#include <sys/socket.h>

#include "src/addr.h"


/* Construct an address from a plain `sockaddr` struct. */
void twist__addr_load(struct twist__addr * addr,
                      const struct sockaddr * sockaddr, socklen_t socklen) {
    memset(addr, 0, sizeof(struct twist__addr));
    memcpy(addr, sockaddr, (size_t) socklen);
    addr->len = (uint16_t) socklen;
}


/* Copy the value of the address `from` into `addr`. */
void twist__addr_copy(struct twist__addr * addr, const struct twist__addr * from) {
    memcpy(addr, from, sizeof(struct twist__addr));
}
