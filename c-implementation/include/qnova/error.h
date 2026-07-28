#ifndef QNOVA_ERROR_H
#define QNOVA_ERROR_H

#include <stddef.h>

#define QNOVA_ERROR_CODE_MIN 1u
#define QNOVA_ERROR_CODE_MAX 9999u

typedef struct {
    unsigned int code;
    const char *file;
    size_t line;
    size_t column;
    const char *message;
} qnova_error_t;

/*
 * Formats:
 * QNOVA-E#### file:line:column message
 *
 * Returns the number of bytes written, excluding the final NUL.
 * Returns -1 for invalid input or insufficient output capacity.
 */
int qnova_error_format(
    char *buffer,
    size_t capacity,
    const qnova_error_t *error
);

#endif
