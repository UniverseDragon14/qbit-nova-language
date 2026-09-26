#include "qnova/qnova.h"
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(FILE *f) {
    fprintf(f,
        "QBIT NOVA Language native v%s\n"
        "usage:\n"
        "  qnova check <file.qnova>\n"
        "  qnova run <file.qnova> [--rng test:<seed>]\n"
        "  qnova compile <file.qnova> -o <file.qbc>\n"
        "  qnova run-qbc <file.qbc> [--rng test:<seed>]\n"
        "  qnova tokens <file.qnova>\n"
        "  qnova ast <file.qnova>\n"
        "  qnova qir <file.qnova>\n"
        "  qnova disasm <file.qbc>\n"
        "  qnova limits\n",
        qnova_version());
}

static int parse_rng(int argc, char **argv, int start, uint64_t *seed, int *det) {
    *seed = 0;
    *det = 0;
    for (int i=start;i<argc;i++) {
        if (!strcmp(argv[i],"--rng") && i+1<argc) {
            const char *v=argv[++i];
            if (strncmp(v,"test:",5)) return -1;
            char *end=NULL;
            errno=0;
            unsigned long long x=strtoull(v+5,&end,10);
            if (errno || !end || *end) return -1;
            *seed=(uint64_t)x;
            *det=1;
        } else return -1;
    }
    return 0;
}

int main(int argc, char **argv) {
    qnova_limits lim=qnova_default_limits();
    qnova_error err={0};
    int rc=0;
    if (argc<2) { usage(stderr); return 2; }
    if (!strcmp(argv[1],"limits")) {
        printf("source_bytes=%zu\n",lim.source_bytes);
        printf("tokens=%zu\n",lim.tokens);
        printf("ast_nodes=%zu\n",lim.ast_nodes);
        printf("call_depth=%zu\n",lim.call_depth);
        printf("repeat_iterations=%zu\n",lim.repeat_iterations);
        printf("qbits=%zu\n",lim.qbits);
        printf("qbc_instructions=%zu\n",lim.qbc_instructions);
        printf("execution_steps=%zu\n",lim.execution_steps);
        return 0;
    }
    if (argc<3) { usage(stderr); return 2; }

    if (!strcmp(argv[1],"check")) rc=qnova_check_file(argv[2],&lim,&err);
    else if (!strcmp(argv[1],"tokens")) rc=qnova_dump_tokens(argv[2],&lim,stdout,&err);
    else if (!strcmp(argv[1],"ast")) rc=qnova_dump_ast(argv[2],&lim,stdout,&err);
    else if (!strcmp(argv[1],"qir")) rc=qnova_dump_qir(argv[2],&lim,stdout,&err);
    else if (!strcmp(argv[1],"disasm")) rc=qnova_disasm_qbc(argv[2],&lim,stdout,&err);
    else if (!strcmp(argv[1],"compile")) {
        if (argc!=5 || strcmp(argv[3],"-o")) { usage(stderr); return 2; }
        rc=qnova_compile_file(argv[2],argv[4],&lim,&err);
    } else if (!strcmp(argv[1],"run") || !strcmp(argv[1],"run-qbc")) {
        uint64_t seed=0; int det=0;
        if (parse_rng(argc,argv,3,&seed,&det)) { usage(stderr); return 2; }
        rc=!strcmp(argv[1],"run")
            ? qnova_run_file(argv[2],&lim,seed,det,stdout,&err)
            : qnova_run_qbc(argv[2],&lim,seed,det,stdout,&err);
    } else {
        usage(stderr);
        return 2;
    }
    if (rc) qnova_print_error(stderr,&err);
    return rc ? 1 : 0;
}
