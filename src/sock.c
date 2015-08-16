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

#include <nectar.h>

#include "src/endian.h"
#include "src/env.h"
#include "src/mem.h"
#include "src/sock.h"


/* Static functions. */
static int handle_tick(struct twist__sock * sock, int64_t now);
static int handle_recv(struct twist__sock * sock,
                       const struct sockaddr * addr, socklen_t addrlen,
                       const uint8_t * payload, size_t len, int64_t now);

static int handle_connect(struct twist__sock * sock,
                          const struct sockaddr * addr, socklen_t addrlen,
                          const uint8_t * payload, size_t len, int64_t now);

static int generate_ticket(struct twist__sock * sock, uint8_t dst[64],
                           const struct sockaddr * addr, socklen_t addrlen, int64_t now);
static int64_t check_ticket(struct twist__sock * sock, const uint8_t src[64],
                            const struct sockaddr * addr, socklen_t addrlen, int64_t now);

static int generate_cookie(struct twist__sock * sock, uint64_t * cookie);


/* Allocate and initialize a new socket. */
int twist__sock_create(struct twist__sock ** sockptr, struct twist__env * env) {
    struct twist__sock * sock;
    uint8_t seed[16];
    int ret;

    /* Allocate the socket struct itself. */
    sock = twist__malloc(sizeof(*sock));
    if (sock == NULL) {
        ret = TWIST_ENOMEM;
        goto err0;
    }

    /* Move `env` into the socket. */
    memcpy(&sock->env, env, sizeof(*env));

    /* Initialize the PRNG. */
    ret = twist__prng_init(&sock->prng, &sock->env);
    if (ret != TWIST_OK)
        goto err1;

    /* Initialize the packet pool. */
    twist__pool_init(&sock->pool);

    /* Initialize the token register. */
    ret = twist__register_init(&sock->reg, 60);
    if (ret != TWIST_OK)
        goto err2;

    /* Initialize the connection hash map. */
    ret = twist__prng_read(&sock->prng, seed, sizeof(seed));
    if (ret != TWIST_OK)
        goto err3;

    ret = twist__dict_init(&sock->dict, seed);
    if (ret != TWIST_OK)
        goto err3;

    /* Initialize the connection min-heap. */
    ret = twist__heap_init(&sock->heap);
    if (ret != TWIST_OK)
        goto err4;

    /* Generate the handshake ticket key. */
    ret = twist__prng_read(&sock->prng, sock->ticket_key, 32);
    if (ret != TWIST_OK)
        goto err5;

    /* Set all other internal fields. */
    sock->last_tick = 0;
    sock->next_tick = 0;
    sock->lingering = NULL;

    /* Finally, update `sockptr` and exit. */
    *sockptr = sock;
    return TWIST_OK;

    /* Error handling. */
err5:
    twist__heap_clear(&sock->heap);
err4:
    twist__dict_clear(&sock->dict);
err3:
    twist__register_clear(&sock->reg);
err2:
    twist__pool_clear(&sock->pool);
    twist__prng_clear(&sock->prng);
err1:
    twist__free(sock);
err0:
    *sockptr = NULL;
    return ret;
}


/* Free a socket. Fails with TWIST_EAGAIN if the socket in question has any
 * open (as in not yet dropped) connections. */
int twist__sock_destroy(struct twist__sock ** sockptr) {
    struct twist__sock * sock;

    /* Dereference the pointer. */
    sock = *sockptr;

    /* Abort if there are any non-dropped connections. */
    if (twist__heap_peek(&sock->heap) != NULL)
        return TWIST_EAGAIN;

    /* Tear down all internal structs. */
    twist__heap_clear(&sock->heap);
    twist__dict_clear(&sock->dict);
    twist__register_clear(&sock->reg);
    twist__pool_clear(&sock->pool);
    twist__prng_clear(&sock->prng);

    /* Free the socket and clear `sockptr`. */
    twist__free(sock);
    *sockptr = NULL;

    return TWIST_OK;
}


/* Feed a clock tick to the socket. */
int twist__sock_tick(struct twist__sock * sock, int64_t now) {
    struct twist__conn * conn;
    int ret;

    /* Let the `tick` function do its job. It was separated out because while
     * the function for receiving packets also needs to process ticks, we don't
     * want to cull the object pool twice. */
    ret = handle_tick(sock, now);

    /* Cull excess objects from the object pool, regardless of whether the
     * `handle_tick` call was successful.
     *
     * TODO: Give the user an opportunity to specify how many objects to keep
     *       in the pool. Eight feels like a reasonable number for now. */
    twist__pool_cull(&sock->pool, 8);

    /* Update `sock->next_tick`. */
    conn = twist__heap_peek(&sock->heap);
    if (conn != NULL) {
        sock->next_tick = conn->next_tick;
    } else {
        sock->next_tick = 0;
    }

    return ret;
}


/* Feed an incoming packet to the socket. */
int twist__sock_recv(struct twist__sock * sock,
                     const struct sockaddr * addr, socklen_t addrlen,
                     const uint8_t * payload, size_t len, int64_t now) {
    struct twist__conn * conn;
    int ret;

    /* Trigger all pending connection timers first. Only if that operation
     * succeeds do we actually process the packet. */
    ret = handle_tick(sock, now);
    if (ret >= 0)
        ret = handle_recv(sock, addr, addrlen, payload, len, now);

    /* Cull excess objects from the object pool, regardless of whether the
     * `handle_tick` and `handle_recv` calls were successful.
     *
     * TODO: Give the user an opportunity to specify how many objects to keep
     *       in the pool. Eight feels like a reasonable number for now. */
    twist__pool_cull(&sock->pool, 8);

    /* Update `sock->next_tick`. */
    conn = twist__heap_peek(&sock->heap);
    if (conn != NULL) {
        sock->next_tick = conn->next_tick;
    } else {
        sock->next_tick = 0;
    }

    return ret;
}


/* Feed a clock tick to the socket (inner). */
static int handle_tick(struct twist__sock * sock, int64_t now) {
    struct twist__packet * pkt;
    struct twist__conn * conn;
    int ret;

    /* Free any packets sent in the previous socket function call. */
    while (sock->lingering != NULL) {
        pkt = sock->lingering;
        sock->lingering = pkt->next;

        twist__pool_free(&sock->pool, pkt);
    }

    /* Time travel is strictly forbidden. */
    if (now < sock->last_tick)
        return TWIST_EINVAL;

    /* Exit early if this tick occurred before the next timer is set to expire,
     * or if there simply aren't any pending timers. */
    if (now < sock->next_tick || sock->next_tick <= 0)
        goto discard;

    /* Propagate this tick to all relevant connections. */
    for (;;) {
        conn = twist__heap_peek(&sock->heap);

        /* The heap is only empty when there aren't any open connections on the
         * socket, which means there can't be any pending timers. */
        if (conn == NULL) {
            sock->next_tick = 0;
            break;
        }

        /* If the next timer isn't due to expire yet, stop. */
        if (conn->next_tick > now) {
            sock->next_tick = conn->next_tick;
            break;
        }

        /* Forward the tick to the next connection. */
        ret = twist__conn_tick(conn, now);
        if (ret != TWIST_OK)
            return ret;

        /* Update `conn`'s position in the heap. */
        twist__heap_fix(&sock->heap, conn);
    }

    /* Everything went fine - store this tick. */
    sock->last_tick = now;

discard:
    return TWIST_OK;
}


/* Feed an incoming packet to the socket (inner). */
static int handle_recv(struct twist__sock * sock,
                       const struct sockaddr * addr, socklen_t addrlen,
                       const uint8_t * payload, size_t len, int64_t now) {
    struct twist__conn * conn;
    struct twist__packet * pkt;
    char type;
    uint64_t cookie;
    int ret;

    /* Discard clearly invalid packets immediately. */
    if (len < 24)
        goto discard;

    /* Decode the destination connection cookie. */
    cookie = be64dec(payload);
    type = '\x00';

    /* Zero cookies are used to indicate control packets, which need to be
     * handled differently than ordinary data packets. */
    if (cookie == 0) {
        /* Validate the version string. */
        if (memcmp(payload + 7, "twist/0", 7) != 0)
            goto discard;

        /* The packet type is indicated by an ASCII character 15 bytes into
         * the packet payload. */
        type = (char) payload[15];
        cookie = be64dec(payload + 16);

        /* Client handshakes are handled by the socket itself, while server
         * and rendezvous handshakes should be forwarded to the relevant
         * connection. */
        if (type == 'h' && cookie == 0)
            return handle_connect(sock, addr, addrlen, payload, len, now);

        /* Discard invalid packets. All other control packets are handled by
         * the receiving connection, which means we fall through here. */
        if (type != 'h' && type != 't')
            goto discard;
    }

    /* Find the connection with this local cookie. If there is none,
     * we discard the packet. */
    conn = twist__dict_find(&sock->dict, cookie);
    if (conn == NULL)
        goto discard;

    /* Construct a proper packet object that we can hand over to the
     * connection state machine. */
    pkt = twist__pool_alloc(&sock->pool);
    if (pkt == NULL)
        return TWIST_ENOMEM;

    twist__packet_init(pkt, addr, addrlen, payload, len);

    /* Pass the packet on to the receiving connection's handle. */
    ret = twist__conn_recv(conn, type, pkt, now);
    if (ret != TWIST_OK)
        return ret;

    /* Update `conn`'s position in the heap. */
    twist__heap_fix(&sock->heap, conn);

discard:
    return TWIST_OK;
}


/* Respond to a client handshake packet. */
static int handle_connect(struct twist__sock * sock,
                          const struct sockaddr * addr, socklen_t addrlen,
                          const uint8_t * payload, size_t len, int64_t now) {
    /* TODO: Everything. */
    return TWIST_OK;
}


/* Generate a handshake ticket. */
static int generate_ticket(struct twist__sock * sock, uint8_t dst[64],
                           const struct sockaddr * addr, socklen_t addrlen, int64_t now) {
    struct nectar_chacha20_ctx chacha;
    struct nectar_hmac_sha512_ctx hmac;
    uint8_t key[32];
    uint32_t token[2];
    int ret;

    /* Grab a 192-bit initialization vector. */
    ret = twist__prng_read(&sock->prng, dst, 24);
    if (ret != TWIST_OK)
        return ret;

    /* Ask the strike register for a fresh token. */
    ret = twist__register_reserve(&sock->reg, token, now);
    if (ret != TWIST_OK)
        return ret;

    memcpy(dst + 24, token, 8);

    /* Encrypt the token. */
    nectar_hchacha20(key, sock->ticket_key, dst);
    nectar_chacha20_init(&chacha, key, dst + 16);
    nectar_chacha20_xor(&chacha, dst + 24, dst + 24, 8);

    /* Sign the ticket. */
    nectar_hmac_sha512_init(&hmac, sock->ticket_key, 32);
    nectar_hmac_sha512_update(&hmac, (const uint8_t *) addr, (size_t) addrlen);
    nectar_hmac_sha512_update(&hmac, (const uint8_t *) dst, 32);
    nectar_hmac_sha512_final(&hmac, dst + 32, 32);

    return TWIST_OK;
}


/* Validate a handshake ticket. */
static int64_t check_ticket(struct twist__sock * sock, const uint8_t src[64],
                            const struct sockaddr * addr, socklen_t addrlen, int64_t now) {
    struct nectar_chacha20_ctx chacha;
    struct nectar_hmac_sha512_ctx hmac;
    uint8_t digest[32];
    uint8_t key[32];
    uint32_t token[2];

    /* Calculate the expected HMAC-SHA512 digest. */
    nectar_hmac_sha512_init(&hmac, sock->ticket_key, 32);
    nectar_hmac_sha512_update(&hmac, (const uint8_t *) addr, (size_t) addrlen);
    nectar_hmac_sha512_update(&hmac, src, 32);
    nectar_hmac_sha512_final(&hmac, digest, 32);

    /* Validate the digest. */
    if (nectar_bcmp(src + 32, digest, 32) != 0)
        return TWIST_EINVAL;

    /* Decrypt the 64-bit token. */
    nectar_hchacha20(key, sock->ticket_key, src);
    nectar_chacha20_init(&chacha, key, src + 16);
    nectar_chacha20_xor(&chacha, (uint8_t *) token, src + 24, 8);

    /* Finally, find the token's position in the bitset. */
    return twist__register_check(&sock->reg, token, now);
}


/* Generate a random connection cookie. */
static int generate_cookie(struct twist__sock * sock, uint64_t * dst) {
    uint64_t cookie;
    int ret;

    /* Keep generating random cookies until we end up with one that is
     * both a) valid and b) available. */
    do {
        ret = twist__prng_read(&sock->prng, (uint8_t *) &cookie, sizeof(cookie));
        if (ret != TWIST_OK)
            return ret;
    } while (cookie == 0 || twist__dict_find(&sock->dict, cookie) != NULL);

    *dst = cookie;
    return TWIST_OK;
}
