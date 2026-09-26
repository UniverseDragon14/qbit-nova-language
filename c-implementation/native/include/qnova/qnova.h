#ifndef QNOVA_QNOVA_H
#define QNOVA_QNOVA_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define QNOVA_VERSION_MAJOR 0
#define QNOVA_VERSION_MINOR 1

typedef struct {
    size_t source_bytes;
    size_t tokens;
    size_t ast_nodes;
    size_t call_depth;
    size_t repeat_iterations;
    size_t qbits;
    size_t qbc_instructions;
    size_t execution_steps;
} qnova_limits;

typedef struct {
    int code;
    const char *file;
    uint32_t line;
    uint32_t column;
    char message[192];
} qnova_error;

qnova_limits qnova_default_limits(void);
const char *qnova_version(void);

int qnova_check_file(const char *path, const qnova_limits *limits, qnova_error *err);
int qnova_compile_file(const char *src_path, const char *out_path,
                       const qnova_limits *limits, qnova_error *err);
int qnova_run_file(const char *path, const qnova_limits *limits,
                   uint64_t rng_seed, int deterministic_rng,
                   FILE *out, qnova_error *err);
int qnova_run_qbc(const char *path, const qnova_limits *limits,
                  uint64_t rng_seed, int deterministic_rng,
                  FILE *out, qnova_error *err);
int qnova_dump_tokens(const char *path, const qnova_limits *limits,
                      FILE *out, qnova_error *err);
int qnova_dump_ast(const char *path, const qnova_limits *limits,
                   FILE *out, qnova_error *err);
int qnova_dump_qir(const char *path, const qnova_limits *limits,
                   FILE *out, qnova_error *err);
int qnova_disasm_qbc(const char *path, const qnova_limits *limits,
                     FILE *out, qnova_error *err);

void qnova_print_error(FILE *out, const qnova_error *err);

#endif
