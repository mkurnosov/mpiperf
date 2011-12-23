/*
 * mempool.c: Memory management routines.
 *
 * Copyright (C) 2010 Mikhail Kurnosov
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  /* for memset */

#include "mempool.h"
#include "mpiperf.h"
#include "logger.h"

enum {
    MEMPOOL_CACHELINE_SIZE = 64,
    MEMPOOL_BLOCKS = 4
};

struct mempool {
    char *pool;
    char *pool_aligned;
    char *end;
    char *ptr;
    int cachedefeat;
    size_t size;
};

static int mempool_alignment = 32;

static inline void *alignptr(void *p, int alignment);

/* alignptr: Aligns pointer on given boundary. */
static inline void *alignptr(void *p, int alignment)
{
    /* (((uintptr_t)p + alignment - 1) & ~(uintptr_t)(alignment - 1)); */
    return (void *)(((unsigned long)p + alignment - 1) & ~(alignment - 1));
}

/* mempool_create: */
mempool_t *mempool_create(size_t allocmax, int cachedefeat)
{
    mempool_t *p;

    if ((p = malloc(sizeof(*p))) == NULL) {
        return NULL;
    }

    if (cachedefeat) {
        /*
         * Allocate memory for circular buffer:
         * block | cache line | block | cache line | ...
         */
        p->size = allocmax * MEMPOOL_BLOCKS +
                  (MEMPOOL_BLOCKS - 1) * MEMPOOL_CACHELINE_SIZE;
        p->size += mempool_alignment - 1;
    } else {
        p->size = allocmax;
    }
    if ( (p->pool = malloc(sizeof(*p->pool) * p->size)) == NULL) {
        free(p);
        return NULL;
    }
   	p->pool_aligned = (char *)alignptr(p->pool, mempool_alignment);
    p->ptr = p->pool_aligned;
    p->end = p->pool + p->size;
    p->cachedefeat = cachedefeat;
    logger_log("Memory pool: size %u bytes, cache defeat: %d",
               p->size, cachedefeat);
    memset(p->pool, 0x77, p->size);
    return p;
}

void mempool_free(mempool_t *mempool)
{
    if (mempool) {
        free(mempool->pool);
    }
    free(mempool);
}

/* mempool_alloc: Returns aligned pointer to memory block of given size. */
void *mempool_alloc(mempool_t *mempool, size_t size)
{
    if (!mempool)
        return NULL;

	if (mempool->cachedefeat) {
	    /* Defeat CPU cache */
	    char *alignedptr = alignptr(mempool->ptr, mempool_alignment);
        if (alignedptr + size < mempool->end) {
            mempool->ptr = alignedptr + size + MEMPOOL_CACHELINE_SIZE;
            return alignedptr;
        }
        mempool->ptr = mempool->pool_aligned + size + MEMPOOL_CACHELINE_SIZE;
	    return (mempool->pool_aligned + size < mempool->end) ?
	           mempool->pool_aligned : NULL;
	}
	/* CPU cache is off: return buffer alloced by malloc */
	return mempool->pool;
}
