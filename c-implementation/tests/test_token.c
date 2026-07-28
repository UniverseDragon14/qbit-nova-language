#include "qnova/token.h"

#include <stdio.h>
#include <string.h>

static int expect_name(
    qnova_token_kind_t kind,
    const char *expected
) {
    const char *actual = qnova_token_kind_name(kind);

    if (strcmp(actual, expected) != 0) {
        printf(
            "FAIL: TOKEN_NAME_MISMATCH expected=%s actual=%s\n",
            expected,
            actual
        );
        return 0;
    }

    return 1;
}

int main(void) {
    qnova_token_t token = {
        .kind = QNOVA_TOKEN_KW_QBIT,
        .byte_offset = 0u,
        .byte_length = 4u,
        .line = 1u,
        .column = 1u
    };

    if (!expect_name(QNOVA_TOKEN_KW_QBIT, "KW_QBIT") ||
        !expect_name(QNOVA_TOKEN_KET_ZERO, "KET_ZERO") ||
        !expect_name(QNOVA_TOKEN_KW_GUARD, "KW_GUARD") ||
        !expect_name(QNOVA_TOKEN_ARROW, "ARROW")) {
        return 1;
    }

    if (!qnova_token_is_valid(&token)) {
        puts("FAIL: VALID_TOKEN_REJECTED");
        return 1;
    }

    token.line = 0u;

    if (qnova_token_is_valid(&token)) {
        puts("FAIL: ZERO_LINE_ACCEPTED");
        return 1;
    }

    token.line = 1u;
    token.kind = QNOVA_TOKEN_KIND_COUNT;

    if (qnova_token_is_valid(&token)) {
        puts("FAIL: INVALID_KIND_ACCEPTED");
        return 1;
    }

    if (strcmp(
            qnova_token_kind_name(QNOVA_TOKEN_KIND_COUNT),
            "INVALID_TOKEN"
        ) != 0) {
        puts("FAIL: INVALID_KIND_NAME_NOT_DETERMINISTIC");
        return 1;
    }

    puts("TOKEN=KW_QBIT line=1 column=1 bytes=4");
    puts("PASS: QNOVA_DETERMINISTIC_TOKEN_MODEL_V01");
    return 0;
}
