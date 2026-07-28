#include "qnova/token.h"

static const char *const qnova_token_names[QNOVA_TOKEN_KIND_COUNT] = {
    [QNOVA_TOKEN_EOF] = "EOF",
    [QNOVA_TOKEN_NEWLINE] = "NEWLINE",
    [QNOVA_TOKEN_SEMICOLON] = "SEMICOLON",

    [QNOVA_TOKEN_IDENTIFIER] = "IDENTIFIER",
    [QNOVA_TOKEN_INTEGER] = "INTEGER",
    [QNOVA_TOKEN_REAL] = "REAL",
    [QNOVA_TOKEN_STRING] = "STRING",

    [QNOVA_TOKEN_KW_LET] = "KW_LET",
    [QNOVA_TOKEN_KW_FN] = "KW_FN",
    [QNOVA_TOKEN_KW_RETURN] = "KW_RETURN",
    [QNOVA_TOKEN_KW_EMIT] = "KW_EMIT",
    [QNOVA_TOKEN_KW_WHEN] = "KW_WHEN",
    [QNOVA_TOKEN_KW_ELSE] = "KW_ELSE",
    [QNOVA_TOKEN_KW_REPEAT] = "KW_REPEAT",
    [QNOVA_TOKEN_KW_GUARD] = "KW_GUARD",
    [QNOVA_TOKEN_KW_OBSERVE] = "KW_OBSERVE",
    [QNOVA_TOKEN_KW_SIMULATE] = "KW_SIMULATE",
    [QNOVA_TOKEN_KW_PATCH] = "KW_PATCH",

    [QNOVA_TOKEN_KW_QBIT] = "KW_QBIT",
    [QNOVA_TOKEN_KW_H] = "KW_H",
    [QNOVA_TOKEN_KW_X] = "KW_X",
    [QNOVA_TOKEN_KW_Z] = "KW_Z",
    [QNOVA_TOKEN_KW_CNOT] = "KW_CNOT",
    [QNOVA_TOKEN_KW_MEASURE] = "KW_MEASURE",
    [QNOVA_TOKEN_KW_RESET] = "KW_RESET",
    [QNOVA_TOKEN_KW_PROBS] = "KW_PROBS",

    [QNOVA_TOKEN_KW_TRUE] = "KW_TRUE",
    [QNOVA_TOKEN_KW_FALSE] = "KW_FALSE",
    [QNOVA_TOKEN_KW_AND] = "KW_AND",
    [QNOVA_TOKEN_KW_OR] = "KW_OR",
    [QNOVA_TOKEN_KW_NOT] = "KW_NOT",

    [QNOVA_TOKEN_KW_INT] = "KW_INT",
    [QNOVA_TOKEN_KW_REAL] = "KW_REAL",
    [QNOVA_TOKEN_KW_BOOL] = "KW_BOOL",
    [QNOVA_TOKEN_KW_STRING] = "KW_STRING",
    [QNOVA_TOKEN_KW_BIT] = "KW_BIT",
    [QNOVA_TOKEN_KW_UNIT] = "KW_UNIT",

    [QNOVA_TOKEN_LEFT_PAREN] = "LEFT_PAREN",
    [QNOVA_TOKEN_RIGHT_PAREN] = "RIGHT_PAREN",
    [QNOVA_TOKEN_LEFT_BRACE] = "LEFT_BRACE",
    [QNOVA_TOKEN_RIGHT_BRACE] = "RIGHT_BRACE",
    [QNOVA_TOKEN_COMMA] = "COMMA",
    [QNOVA_TOKEN_COLON] = "COLON",

    [QNOVA_TOKEN_ARROW] = "ARROW",
    [QNOVA_TOKEN_ASSIGN] = "ASSIGN",
    [QNOVA_TOKEN_EQUAL] = "EQUAL",
    [QNOVA_TOKEN_NOT_EQUAL] = "NOT_EQUAL",
    [QNOVA_TOKEN_LESS] = "LESS",
    [QNOVA_TOKEN_LESS_EQUAL] = "LESS_EQUAL",
    [QNOVA_TOKEN_GREATER] = "GREATER",
    [QNOVA_TOKEN_GREATER_EQUAL] = "GREATER_EQUAL",

    [QNOVA_TOKEN_PLUS] = "PLUS",
    [QNOVA_TOKEN_MINUS] = "MINUS",
    [QNOVA_TOKEN_STAR] = "STAR",
    [QNOVA_TOKEN_SLASH] = "SLASH",
    [QNOVA_TOKEN_PERCENT] = "PERCENT",

    [QNOVA_TOKEN_KET_ZERO] = "KET_ZERO",
    [QNOVA_TOKEN_KET_ONE] = "KET_ONE"
};

const char *qnova_token_kind_name(qnova_token_kind_t kind) {
    if (kind < QNOVA_TOKEN_EOF || kind >= QNOVA_TOKEN_KIND_COUNT) {
        return "INVALID_TOKEN";
    }

    if (qnova_token_names[kind] == NULL) {
        return "INVALID_TOKEN";
    }

    return qnova_token_names[kind];
}

int qnova_token_is_valid(const qnova_token_t *token) {
    if (token == NULL) {
        return 0;
    }

    if (token->kind < QNOVA_TOKEN_EOF ||
        token->kind >= QNOVA_TOKEN_KIND_COUNT) {
        return 0;
    }

    if (token->line == 0u || token->column == 0u) {
        return 0;
    }

    if (token->kind != QNOVA_TOKEN_EOF &&
        token->kind != QNOVA_TOKEN_NEWLINE &&
        token->byte_length == 0u) {
        return 0;
    }

    return 1;
}
