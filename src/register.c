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

#include "src/mem.h"
#include "src/register.h"


/* Conversion between seconds and nanoseconds. */
#define nstos(x)  ((x) / 1000000000)
#define stons(x)  ((x) * 1000000000)


/* Minimum (default) and maximum bitset sizes, measured in 32-bit blocks. */
#define MIN_BITS_SIZE  (1 <<  5)
#define MAX_BITS_SIZE  (1 << 29)


/* Static functions. */
static int reduce(struct twist__register * reg, uint32_t horizon, uint32_t current);
static int resize(struct twist__register * reg, uint32_t size,
                  uint32_t head, uint32_t tail, uint32_t horizon);


/* Initialize the register. Returns TWIST_ENOMEM if a necessary allocation
 * failed, otherwise TWIST_OK. */
int twist__register_init(struct twist__register * reg, uint32_t lifetime) {
    uint32_t * offsets;
    uint32_t * bits;

    /* Allocate some memory for our circular arrays. */
    offsets = twist__malloc(lifetime * sizeof(*offsets));
    if (offsets == NULL)
        return TWIST_ENOMEM;

    bits = twist__malloc(sizeof(*bits) * MIN_BITS_SIZE);
    if (bits == NULL) {
        twist__free(offsets);
        return TWIST_ENOMEM;
    }

    /* Whenever `reg->counter == 0`, all offsets should be zero. */
    memset(offsets, 0, lifetime * sizeof(*offsets));

    /* Initialize the register's fields. */
    reg->offsets = offsets;
    reg->lifetime = lifetime;
    reg->cursor = 0;
    reg->counter = 0;

    reg->bits = bits;
    reg->size = MIN_BITS_SIZE;
    reg->mask = MIN_BITS_SIZE - 1;

    return TWIST_OK;
}


/* Free all heap memory managed by the register. */
void twist__register_clear(struct twist__register * reg) {
    twist__free(reg->offsets);
    twist__free(reg->bits);
}


/* Generate a new token. Returns TWIST_ENOMEM if the underlying storage array
 * is full and an attempt to grow it failed, or TWIST_EAGAIN if we've already
 * reached the hard limit of tokens generated per second (2,147,483,647,
 * or 2^31 - 1); otherwise TWIST_OK. */
int twist__register_reserve(struct twist__register * reg, uint32_t token[2], int64_t now) {
    uint32_t current, horizon;
    uint32_t head, tail;
    int ret;

    /* Find the current bucket, as well as the oldest still valid bucket. */
    current = (uint32_t) nstos(now);
    horizon = (current - reg->lifetime) + 1;
    if (horizon >= current)
        horizon = 0;

    /* If this token's bit will live at the same offset as the previous
     * token's, all work has already been done for us. */
    if (reg->cursor == current && (reg->counter & 31) != 0)
        goto done;

    /* Expire old tokens, potentially freeing up some room. */
    ret = reduce(reg, horizon, current);
    if (ret != TWIST_OK)
        return ret;

    /* If the register is empty we can get away with doing very little. */
    if (reg->counter == 0) {
        reg->cursor = current;
        reg->counter = 0;
        reg->bits[0] = 0;

        goto done;
    }

    /* Make sure `reg->counter` doesn't overflow. */
    if (reg->counter == 0xffffffff - 31)
        return TWIST_EAGAIN;

    /* Find the starting index of the occupied portion of `reg->bits`, and the
     * index immediately after its end. The latter also happens to be where this
     * token's bit will live. */
    head = reg->offsets[horizon % reg->lifetime];
    tail = (reg->offsets[reg->cursor % reg->lifetime] + (reg->counter-1)/32 + 1) & reg->mask;

    /* Grow `reg->bits` if it's full. */
    if (head == tail) {
        if (reg->size == MAX_BITS_SIZE)
            return TWIST_ENOMEM;

        ret = resize(reg, 2*reg->size, head, (tail > 0 ? tail : reg->size), horizon);
        if (ret != TWIST_OK)
            return ret;

        /* Because the call to `resize` may have modified `reg->offsets` we
         * have to calculate `tail` once again (`head` is also potentially
         * incorrect, but we don't need it anymore). */
        tail = (reg->offsets[reg->cursor % reg->lifetime] + (reg->counter-1)/32 + 1) & reg->mask;
    }

    /* If this is the first token in a new bucket, reset the counter. Also
     * forward `reg->cursor` to its new value, updating the offsets of all
     * buckets as we go. */
    if (reg->cursor < current) {
        reg->counter = 0;

        do {
            reg->cursor++;
            reg->offsets[reg->cursor % reg->lifetime] = tail;
        } while (reg->cursor < current);
    }

    /* Clear the next 32 bits. */
    reg->bits[tail] = 0;

    /* Encode the token. */
done:
    token[0] = current;
    token[1] = reg->counter++;

    return TWIST_OK;
}


/* Claim a token, removing it from the register. Returns TWIST_EINVAL if the
 * token has expired or has already been claimed; otherwise TWIST_OK. */
int twist__register_claim(struct twist__register * reg, const uint32_t token[2], int64_t now) {
    uint32_t upper, lower;
    uint32_t bucket, index;
    uint32_t offset, bit;

    /* Pluck the token's components. */
    bucket = token[0];
    index = token[1];

    /* Calculate the lower and upper limits of valid bucket values. */
    upper = (uint32_t) nstos(now);
    lower = (upper >= reg->lifetime ? upper - reg->lifetime : 0);

    if (!(lower <= bucket && bucket <= upper))
        return TWIST_EINVAL;

    /* Find the token's bit's position; if this bit is set the token has
     * already been claimed. */
    offset = (reg->offsets[bucket % reg->lifetime] + (index / 32)) & reg->mask;
    bit = 1 << (index & 31);

    if ((reg->bits[offset] & bit))
        return TWIST_EINVAL;

    /* Mark this token as claimed. */
    reg->bits[offset] |= bit;

    return TWIST_OK;
}


/* Remove any expired tokens from the register, then shrink its internal storage
 * array if possible. Returns TWIST_ENOMEM if a necessary allocation failed,
 * otherwise TWIST_OK. */
int twist__register_reduce(struct twist__register * reg, int64_t now) {
    uint32_t current, horizon;

    /* Find the current bucket, as well as the oldest still valid bucket. */
    current = (uint32_t) nstos(now);
    horizon = (current - reg->lifetime) + 1;
    if (horizon >= current)
        horizon = 0;

    return reduce(reg, horizon, current);
}


/* Shrink the register's backing bitset if possible. */
static int reduce(struct twist__register * reg, uint32_t horizon, uint32_t current) {
    uint32_t head, tail;
    uint32_t size, used;

    /* Because tokens expire one bucket at a time, only the first call to this
     * function every second has a chance of reclaiming any space. */
    if (reg->cursor == current)
        return TWIST_OK;

    /* Empty registers don't need cleaning, and neither do registers that
     * haven't existed long enough to expire any buckets. */
    if (reg->counter == 0 || horizon == 0)
        return TWIST_OK;

    /* If the last bucket has expired, all of them have. */
    if (reg->cursor < horizon) {
        reg->counter = 0;

        /* Zero all offsets again. */
        memset(reg->offsets, 0, reg->lifetime * sizeof(*reg->offsets));

        /* Shrink our bit storage. */
        if (reg->size > MIN_BITS_SIZE)
            return resize(reg, MIN_BITS_SIZE, 0, 1, horizon);

        return TWIST_OK;
    }

    /* Find where the occupied region of `reg->bits` begins and ends. */
    head = reg->offsets[horizon % reg->lifetime];
    tail = (reg->offsets[reg->cursor % reg->lifetime] + (reg->counter-1)/32 + 1) & reg->mask;

    /* Keep halving the size until we reach at least 25% utilization. */
    size = reg->size;
    used = (head < tail ? tail - head : tail + (size - head));

    while (used < size/4 && size > MIN_BITS_SIZE)
        size /= 2;

    if (size != reg->size)
        return resize(reg, size, head, (tail > 0 ? tail : reg->size), horizon);

    return TWIST_OK;
}


/* Resize the register's underlying bitset. */
static int resize(struct twist__register * reg, uint32_t size,
                  uint32_t head, uint32_t tail, uint32_t horizon) {
    uint32_t * bits;
    uint32_t i, j, k;

    /* Use `realloc` when possible to simply truncate or extend the array. */
    if (head < tail && tail <= size) {
        bits = twist__realloc(reg->bits, size * sizeof(*bits));
        if (bits == NULL)
            return TWIST_ENOMEM;

        goto done;
    }

    /* Allocate our new array and copy the interesting parts of `reg->bits`. */
    bits = twist__malloc(size * sizeof(*bits));
    if (bits == NULL)
        return TWIST_ENOMEM;

    for (i = head, j = 0; i < tail; i++)
        bits[j++] = reg->bits[i & reg->mask];

    /* Starting with the oldest valid bucket and ending with the most recent
     * one, update all offsets to what they'll be in the new array. */
    for (i = horizon; i <= reg->cursor; i++) {
        k = i % reg->lifetime;

        if (reg->offsets[k] >= head)
            reg->offsets[k] -= head;
        else
            reg->offsets[k] += j;
    }

    /* Free the old array. */
    twist__free(reg->bits);

    /* Store the new array. */
done:
    reg->bits = bits;
    reg->size = size;
    reg->mask = size - 1;

    return TWIST_OK;
}
