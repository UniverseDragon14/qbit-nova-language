#include "qnova/error.h"

#include <stdio.h>
#include <string.h>

int main(void) {
    static const char expected[] =
        "QNOVA-E1001 hello.qnova:2:3 unexpected token";

    qnova_error_t error = {
        .code = 1001u,
        .file = "hello.qnova",
        .line = 2u,
        .column = 3u,
        .message = "unexpected token"
    };

    char output[128];
    char tiny[8];

    if (qnova_error_format(output, sizeof output, &error) < 0) {
        puts("FAIL: VALID_ERROR_REJECTED");
        return 1;
    }

    if (strcmp(output, expected) != 0) {
        printf("FAIL: FORMAT_MISMATCH\nEXPECTED=%s\nACTUAL=%s\n",
               expected, output);
        return 1;
    }

    error.code = 0u;

    if (qnova_error_format(output, sizeof output, &error) >= 0) {
        puts("FAIL: INVALID_CODE_ACCEPTED");
        return 1;
    }

    error.code = 1001u;

    if (qnova_error_format(tiny, sizeof tiny, &error) >= 0) {
        puts("FAIL: TRUNCATED_ERROR_ACCEPTED");
        return 1;
    }

    printf("OUTPUT=%s\n", expected);
    puts("PASS: QNOVA_DETERMINISTIC_ERROR_ENGINE_V01");
    return 0;
}
