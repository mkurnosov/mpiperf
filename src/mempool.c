/*
 * mempool.c: Memory management routines.
 *
 * Copyright (C) 2010-2012 Mikhail Kurnosov
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  /* for memset */

#include "mempool.h"
#include "mpiperf.h"
#include "logger.h"

enum {
    MEMPOOL_CACHELINE_SIZE = 64,
    MEMPOOL_BLOCKS = 8
};

struct mempool {
    char *pool;				/* Base pointer to mem. pool */
    char *pool_aligned;		/* Aligned pointer to mem. pool */
    char *end;				/* End of mem. pool */
    char *ptr;				/* Current free block */
    size_t size;			/* Pool size */
    int cachedefeat;
};

static int mempool_alignment = 32;

static inline void *alignptr(void *p, int alignment);

/* mempool_create: */
mempool_t *mempool_create(size_t size, int cachedefeat)
{
    mempool_t *p;

    if ((p = malloc(sizeof(*p))) == NULL) {
        return NULL;
    }

    if (size == 0) {
		/*
		 * If the size of the space requested by malloc is zero, the behavior
		 * is implementation defined.
		 * We increase size to 1 byte.
		 */
    	size = 1;
    }

    if (cachedefeat) {
        /*
         * Allocate memory for circular buffer:
         * data block | cache line | data block | cache line | ...
         */
        p->size = size * MEMPOOL_BLOCKS +
                  (MEMPOOL_BLOCKS - 1) * MEMPOOL_CACHELINE_SIZE;
        p->size += mempool_alignment - 1;
    } else {
        p->size = size;
    }

    if ( (p->pool = malloc(sizeof(*p->pool) * p->size)) == NULL) {
        free(p);
        return NULL;
    }
   	p->pool_aligned = (char *)alignptr(p->pool, mempool_alignment);
    p->ptr = p->pool_aligned;
    p->end = p->pool + p->size;
    p->cachedefeat = cachedefeat;
    logger_log("Memory pool: total size %u bytes, cache defeat: %d",
               p->size, p->cachedefeat);
    /* memset(p->pool, 0x77, p->size); */
    return p;
}

void mempool_free(mempool_t *mempool)
{
    if (mempool) {
        free(mempool->pool);
        free(mempool);
    }
}

/* mempool_alloc: Returns aligned pointer to memory block of given size. */
void *mempool_alloc(mempool_t *mempool, size_t size)
{
    if (!mempool)
        return NULL;

    if (size == 0)
    	return 0;

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
	return mempool->pool;
}

/* alignptr: Aligns pointer on given boundary. */
static inline void *alignptr(void *p, int alignment)
{
    /* (((uintptr_t)p + alignment - 1) & ~(uintptr_t)(alignment - 1)); */
    return (void *)(((unsigned long)p + alignment - 1) & ~(alignment - 1));
}
