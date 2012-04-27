/*
 * mempool.h: Memory management routines.
 *
 * Copyright (C) 2010-2012 Mikhail Kurnosov
 */

#ifndef MEMPOOL_H
#define MEMPOOL_H

#include <stdlib.h>

typedef struct mempool mempool_t;

mempool_t *mempool_create(size_t allocmax, int cachedefat);
void mempool_free(mempool_t *mempool);
void *mempool_alloc(mempool_t *mempool, size_t size);

#endif /* MEMPOOL_H */
