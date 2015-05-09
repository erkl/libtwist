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

#ifndef LIBTWINE_ADDR_H
#define LIBTWINE_ADDR_H


/* Maximum number of address bytes that will fit in a `twine__addr`. */
#define MAX_ADDR_LEN  30


/* This struct represents a network address. It basically functions as a much
 * smaller `struct sockaddr_storage`, with a baked-in socklen_t. */
struct twine__addr {
    /* 30 bytes of 8-byte aligned storage. */
    uint64_t storage;
    uint8_t pad[22];

    /* Address size. */
    uint16_t len;
};


/* Construct an address from a plain `sockaddr` struct. */
void twine__addr_load(struct twine__addr * addr,
                      struct sockaddr * sockaddr, socklen_t socklen);

/* Copy the value of the address `from` into `addr`. */
void twine__addr_copy(struct twine__addr * addr, struct twine__addr * from);


#endif
