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

#ifndef LIBTWIST_ENDIAN_H
#define LIBTWIST_ENDIAN_H

#include "include/twist.h"


/* Write a 24-bit integer to dst in big-endian form. */
static inline void be24enc(uint8_t dst[3], uint32_t x) {
    dst[0] = (uint8_t) (x >> 16);
    dst[1] = (uint8_t) (x >> 8);
    dst[2] = (uint8_t) x;
}


/* Read a 24-bit integer from src in big-endian form. */
static inline uint32_t be24dec(const uint8_t src[3]) {
    return ((uint32_t) src[0]) << 16
         | ((uint32_t) src[1]) << 8
         | ((uint32_t) src[2]);
}


/* Write a 32-bit integer to dst in big-endian form. */
static inline void be32enc(uint8_t dst[4], uint32_t x) {
    dst[0] = (uint8_t) (x >> 24);
    dst[1] = (uint8_t) (x >> 16);
    dst[2] = (uint8_t) (x >> 8);
    dst[3] = (uint8_t) x;
}


/* Read a 32-bit integer from src in big-endian form. */
static inline uint32_t be32dec(const uint8_t src[4]) {
    return ((uint32_t) src[0]) << 24
         | ((uint32_t) src[1]) << 16
         | ((uint32_t) src[2]) << 8
         | ((uint32_t) src[3]);
}


/* Write a 64-bit integer to dst in big-endian form. */
static inline void be64enc(uint8_t dst[8], uint64_t x) {
    dst[0] = (uint8_t) (x >> 56);
    dst[1] = (uint8_t) (x >> 48);
    dst[2] = (uint8_t) (x >> 40);
    dst[3] = (uint8_t) (x >> 32);
    dst[4] = (uint8_t) (x >> 24);
    dst[5] = (uint8_t) (x >> 16);
    dst[6] = (uint8_t) (x >> 8);
    dst[7] = (uint8_t) x;
}


/* Read a 64-bit integer from src in big-endian form. */
static inline uint64_t be64dec(const uint8_t src[8]) {
    return ((uint64_t) src[0]) << 56
         | ((uint64_t) src[1]) << 48
         | ((uint64_t) src[2]) << 40
         | ((uint64_t) src[3]) << 32
         | ((uint64_t) src[4]) << 24
         | ((uint64_t) src[5]) << 16
         | ((uint64_t) src[6]) << 8
         | ((uint64_t) src[7]);
}


#endif
