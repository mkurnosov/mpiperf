/*
 * util.c: Utilitary functions.
 *
 * Copyright (C) 2010 Mikhail Kurnosov
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>

#include <mpi.h>

#include "util.h"
#include "mpiperf.h"

enum {
    ERROR_BUF_MAX = 512
};

/* xmalloc: */
void *xmalloc(size_t size)
{
    void *p;

    if (size == 0 )
        return NULL;

    if ( (p = malloc(size)) == NULL) {
        exit_error("xmalloc(): No enough memory");
    }
    return p;
}

/* xrealloc: */
void *xrealloc(void *ptr, size_t size)
{
    void *p;

    if ( (p = realloc(ptr, size)) == NULL) {
        exit_error("xrealloc(): No enough memory");
    }
    return p;
}

/* print_error: Prints an error message. */
void print_error(const char *format, ...)
{
    va_list ap;
    static char buf[ERROR_BUF_MAX];

    if (IS_MASTER_RANK) {
        va_start(ap, format);
        vsprintf(buf, format, ap);
        va_end(ap);

        if (strlen(buf) > 0)
            fprintf(stderr, "%s: error: %s\n", mpiperf_progname, buf);
    }
}

/* exit_error: Prints an error message and terminates all processes. */
void exit_error(const char *format, ...)
{

    va_list ap;
    static char buf[ERROR_BUF_MAX];

    if (IS_MASTER_RANK) {
        va_start(ap, format);
        vsprintf(buf, format, ap);
        va_end(ap);

        if (strlen(buf) > 0) {
            fprintf(stderr, "%s: error: %s\n", mpiperf_progname, buf);
        }
    }
    mpiperf_finalize();
    MPI_Abort(MPI_COMM_WORLD, 1);
    exit(EXIT_FAILURE);
}

/* exit_success: */
void exit_success()
{
    MPI_Finalize();
    /* MPI_Abort(MPI_COMM_WORLD, 0); */
    exit(EXIT_SUCCESS);
}

/*
 * getworldrank: Translate rank of process from communicator comm to
 *              communicator MPI_COMM_WORLD.
 */
int getworldrank(MPI_Comm comm, int rank)
{
    static MPI_Group worldgroup;
    static int isfirstcall = 1;
    int worldrank, isintercomm = 0;
    MPI_Group group;

    MPI_Comm_test_inter(comm, &isintercomm);
    if (isintercomm) {
        MPI_Comm_remote_group(comm, &group);
    } else {
        MPI_Comm_group(comm, &group);
    }

    if (isfirstcall) {
        MPI_Comm_group(MPI_COMM_WORLD, &worldgroup);
        isfirstcall = 0;
    }
    MPI_Group_translate_ranks(group, 1, &rank, worldgroup, &worldrank);
    MPI_Group_free(&group);
    return worldrank;
}

/*
 * parse_intval: Converts string to integer.
 *               Recognizes suffixes KiB, MiB, GiB.
 */
int parse_intval(char *s)
{
    enum { BUFSIZE = 8 };
    char buf[BUFSIZE];
    long int val, scale = 1;
    char *p;
    int i;

    val = strtol(s, &p, 0);
    if (p && p != s) {
        for (; *p == ' ' || *p == '\t'; p++);
        for (i = 0; isalpha(*p); p++) {
            buf[i++] = *p;
        }
        buf[i] = '\0';

        if (strcasecmp(buf, "kib") == 0) {
            scale = 1 << 10;
        } else if (strcasecmp(buf, "mib") == 0) {
            scale = 1 << 20;
        } else if (strcasecmp(buf, "gib") == 0) {
            scale = 1 << 30;
        }
    }
    return val * scale;
}

