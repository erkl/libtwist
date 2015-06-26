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

#include "src/mem.h"
#include "src/sock.h"


/* Allocate and initialize a new socket. */
int twist__sock_create(struct twist__sock ** sockptr, struct twist__env * env) {
    struct twist__sock * sock;
    uint8_t seed[16];
    int ret;

    /* Allocate the socket struct itself. */
    sock = malloc(sizeof(*sock));
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
    ret = twist__env_entropy(&sock->env, seed, sizeof(seed));
    if (ret != TWIST_OK)
        goto err3;

    ret = twist__dict_init(&sock->dict, seed);
    if (ret != TWIST_OK)
        goto err3;

    /* Initialize the connection min-heap. */
    ret = twist__heap_init(&sock->heap);
    if (ret != TWIST_OK)
        goto err4;

    /* Set all other internal fields. */
    sock->last_tick = 0;
    sock->next_tick = 0;

    /* Finally, update `sockptr` and exit. */
    *sockptr = sock;
    return TWIST_OK;

    /* Error handling. */
err4:
    twist__dict_clear(&sock->dict);
err3:
    twist__register_clear(&sock->reg);
err2:
    twist__pool_clear(&sock->pool);
    twist__prng_clear(&sock->prng);
err1:
    free(sock);
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
    free(sock);
    *sockptr = NULL;

    return TWIST_OK;
}


/* Feed a clock tick to the socket. */
int64_t twist__sock_tick(struct twist__sock * sock, int64_t now) {
    struct twist__conn * conn;
    int64_t ret;

    /* Time travel is strictly forbidden. */
    if (now < sock->last_tick)
        return TWIST_EINVAL;

    /* Exit early if this tick occurred "too early". */
    if (now < sock->next_tick || sock->next_tick <= 0)
        return sock->next_tick;

    /* Propagate this tick to all relevant connections. */
    for (;;) {
        conn = twist__heap_peek(&sock->heap);

        /* We're done once we either find ourselves with no connections on the
         * socket, or when all connections have `next_tick` values greater than
         * the current time. */
        if (conn == NULL) {
            ret = 0;
            break;
        } else if (conn->next_tick > now) {
            ret = conn->next_tick;
            break;
        }

        /* Forward the tick to the next connection. */
        ret = twist__conn_tick(conn, now);
        if (ret < 0)
            goto cull;

        /* Make sure the connection's `next_tick` value is up to date,
         * and that the socket's connection heap is ordered. */
        if (ret != conn->next_tick) {
            conn->next_tick = ret;
            twist__heap_fix(&sock->heap, conn);
        }
    }

    /* Update the socket's tick values (Note that we only get here if all
     * `twist__conn_tick` calls went fine. */
    sock->last_tick = now;
    sock->next_tick = ret;

cull:
    /* Cull excess objects from the object pool.
     *
     * TODO: Give the user an opportunity to specify how many objects to keep
     *       in the pool. Eight feels like a reasonable number for now. */
    twist__pool_cull(&sock->pool, 8);

    return ret;
}
