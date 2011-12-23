/*
 * logger.h: Logger.
 *
 * (C) 2009-2010 Mikhail Kurnosov <mkurnosov@gmail.com>
 */

#ifndef LOGGER_H
#define LOGGER_H

extern int logger_rootonly_flag;
extern int logger_debug_flag;

int logger_initialize(char *logfile, int logmaster_only);
int logger_finalize();
int logger_log(const char *format, ...);
void logger_flush();

#endif /* LOGGER_H */

