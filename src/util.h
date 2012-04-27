/*
 * util.h: Utilitary functions.
 *
 * Copyright (C) 2010 Mikhail Kurnosov
 */

#ifndef UTIL_H
#define UTIL_H

#include <mpi.h>

void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);
void print_error(const char *format, ...);
void exit_error(const char *format, ...);
void exit_success();
int getworldrank(MPI_Comm comm, int rank);
MPI_Comm createcomm(MPI_Comm comm, int size);
int parse_intval(char *s);

#endif /* UTIL_H */

