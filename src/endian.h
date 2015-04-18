/* Copyright (c) 2015, Erik Lundin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the copyright holder nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#ifndef LIBTWINE_ENDIAN_H
#define LIBTWINE_ENDIAN_H

#include <stdint.h>


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
