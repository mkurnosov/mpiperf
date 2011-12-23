/*
 * logger.c: Logger.
 *
 * (C) 2009-2010 Mikhail Kurnosov <mkurnosov@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "logger.h"
#include "mpiperf.h"

enum {
    LOGGER_BUF_MAX = 1024,
    PATH_MAX = 1024
};

int logger_rootonly_flag = 0;

static FILE *logger_flog = NULL;

int logger_initialize(char *logfile, int logmaster_only)
{
    char filename[PATH_MAX];

    if (logfile == NULL) {
        return MPIPERF_SUCCESS;
    }
    logger_rootonly_flag = logmaster_only;

    if (!logger_rootonly_flag || (logger_rootonly_flag && IS_MASTER_RANK)) {
        sprintf(filename, "%s.%d", logfile, mpiperf_rank);
        if ( (logger_flog = fopen(filename, "w")) == NULL) {
            return MPIPERF_FAILURE;
        }
        logger_log("%s log opened", mpiperf_progname);
    }
    return MPIPERF_SUCCESS;
}

int logger_finalize()
{
    if (logger_flog != NULL) {
        logger_log("Log closed");
        fclose(logger_flog);
        logger_flog = NULL;
    }
    return MPIPERF_SUCCESS;
}

int logger_log(const char *format, ...)
{
    if (logger_flog == NULL)
        return MPIPERF_FAILURE;

    static char buf[LOGGER_BUF_MAX];
    va_list ap;
    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);

    time_t t;
    time(&t);
    struct tm *tm = localtime(&t);
    static char timebuf[LOGGER_BUF_MAX];
    strftime(timebuf, LOGGER_BUF_MAX, "%Y-%M-%d %H:%M:%S", tm);
    fprintf(logger_flog, "%s: %s\n", timebuf, buf);
    logger_flush();
    return MPIPERF_SUCCESS;
}

void logger_flush()
{
    if (logger_flog == NULL)
        return;
    fflush(logger_flog);
}


