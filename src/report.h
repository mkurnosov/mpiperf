/*
 * report.h: Report module.
 *
 * Copyright (C) 2011 Mikhail Kurnosov
 */

#ifndef REPORT_H
#define REPORT_H

int report_write_header();
void report_printf(const char *format, ...);

#endif /* REPORT_H */
