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

#ifndef LIBTWINE_PACKET_H
#define LIBTWINE_PACKET_H

#include "twine.h"


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
    struct sockaddr addr;
    socklen_t addrlen;

    /* Intrusive list pointers. */
    struct twine__packet * prev;
    struct twine__packet * next;
};


#endif
