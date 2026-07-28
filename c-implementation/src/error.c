#include "qnova/error.h"

#include <stdio.h>

int qnova_error_format(
    char *buffer,
    size_t capacity,
    const qnova_error_t *error
) {
    int written;

    if (buffer == NULL || capacity == 0u || error == NULL) {
        return -1;
    }

    buffer[0] = '\0';

    if (error->file == NULL || error->message == NULL) {
        return -1;
    }

    if (error->code < QNOVA_ERROR_CODE_MIN ||
        error->code > QNOVA_ERROR_CODE_MAX ||
        error->line == 0u ||
        error->column == 0u) {
        return -1;
    }

    written = snprintf(
        buffer,
        capacity,
        "QNOVA-E%04u %s:%zu:%zu %s",
        error->code,
        error->file,
        error->line,
        error->column,
        error->message
    );

    if (written < 0 || (size_t)written >= capacity) {
        buffer[0] = '\0';
        return -1;
    }

    return written;
}
