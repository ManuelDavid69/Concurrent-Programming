#ifndef PC_COMMON_H
#define PC_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

/* ------------------------------------------------------------------------- */
/* Errors and allocation                                                     */
/* ------------------------------------------------------------------------- */

void die(const char *format, ...)
    __attribute__((noreturn, format(printf, 1, 2)));

void *xmalloc(size_t size);
void *xcalloc(size_t count, size_t size);
void *xrealloc(void *pointer, size_t size);
char *xstrdup(const char *string);

/* ------------------------------------------------------------------------- */
/* Parsing and random numbers                                                */
/* ------------------------------------------------------------------------- */

/* Strict integer parsing: the whole string must be a valid int. */
bool parse_int(const char *text, int *value);
bool parse_long(const char *text, long *value);
bool parse_double(const char *text, double *value);

/* Seeds rand() from the clock and the pid. */
void pc_seed(void);
int rand_range(int minimum, int maximum);

/* ------------------------------------------------------------------------- */
/* Timing                                                                    */
/* ------------------------------------------------------------------------- */

/*
 * All the labs that simulate work use these helpers.  PC_SLEEP_SCALE (a
 * positive floating-point number, default 1.0) divides every duration, which
 * keeps the integration tests fast without changing the programs.
 */
double pc_sleep_scale(void);
void pc_sleep(double seconds);
int pc_delay_ms(int milliseconds);

/* ------------------------------------------------------------------------- */
/* File descriptors                                                          */
/* ------------------------------------------------------------------------- */

/* Writes exactly count bytes, retrying on EINTR and partial writes. */
int write_all(int fd, const void *buffer, size_t count);

/* Reads exactly count bytes, retrying on EINTR and partial reads. */
int read_all(int fd, void *buffer, size_t count);

/* Closes a descriptor and sets it to -1. */
void close_fd(int *fd);

/* ------------------------------------------------------------------------- */
/* Processes                                                                 */
/* ------------------------------------------------------------------------- */

pid_t xfork(void);

#endif
