#define _POSIX_C_SOURCE 200809L
#include "qnova/qnova.h"

#include <ctype.h>
#include <complex.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define U32_NONE UINT32_MAX
#define ARRAY_LEN(a) (sizeof(a)/sizeof((a)[0]))

typedef struct {
    uint32_t line, col, end_line, end_col;
} Span;

static void set_error(qnova_error *e, int code, const char *file,
                      uint32_t line, uint32_t col, const char *msg) {
    if (!e || e->code) return;
    e->code=code;
    e->file=file;
    e->line=line ? line : 1;
    e->column=col ? col : 1;
    snprintf(e->message,sizeof(e->message),"%s",msg);
}

void qnova_print_error(FILE *out, const qnova_error *e) {
    if (!e || !e->code) return;
    fprintf(out,"QNOVA-E%04d %s:%u:%u %s\n",
            e->code,e->file?e->file:"<input>",e->line,e->column,e->message);
}

const char *qnova_version(void) { return "0.1-native"; }

qnova_limits qnova_default_limits(void) {
    qnova_limits l={
        .source_bytes=1024*1024,
        .tokens=100000,
        .ast_nodes=100000,
        .call_depth=64,
        .repeat_iterations=1000000,
        .qbits=20,
        .qbc_instructions=200000,
        .execution_steps=10000000,
    };
    return l;
}

static void *xmalloc(size_t n) {
    void *p=malloc(n?n:1);
    if (!p) { fprintf(stderr,"qnova: out of memory\n"); exit(70); }
    return p;
}
static void *xcalloc(size_t n,size_t s) {
    void *p=calloc(n?n:1,s?s:1);
    if (!p) { fprintf(stderr,"qnova: out of memory\n"); exit(70); }
    return p;
}
static void *xrealloc(void *p,size_t n) {
    p=realloc(p,n?n:1);
    if (!p) { fprintf(stderr,"qnova: out of memory\n"); exit(70); }
    return p;
}
static char *xstrdup(const char *s) {
    size_t n=strlen(s)+1;
    char *r=xmalloc(n);
    memcpy(r,s,n);
    return r;
}
static char *xstrndup(const char *s,size_t n) {
    char *r=xmalloc(n+1);
    memcpy(r,s,n);
    r[n]=0;
    return r;
}

static int read_entire_file(const char *path, size_t max,
                            char **out, size_t *out_n, qnova_error *err) {
    FILE *f=fopen(path,"rb");
    if (!f) {
        set_error(err,1001,path,1,1,"invalid source character");
        return -1;
    }
    if (fseek(f,0,SEEK_END)) { fclose(f); return -1; }
    long sz=ftell(f);
    if (sz<0) { fclose(f); return -1; }
    if ((size_t)sz>max) {
        fclose(f);
        set_error(err,1001,path,1,1,"invalid source character");
        return -1;
    }
    rewind(f);
    char *buf=xmalloc((size_t)sz+1);
    size_t got=fread(buf,1,(size_t)sz,f);
    if (got!=(size_t)sz && ferror(f)) {
        fclose(f); free(buf); return -1;
    }
    fclose(f);
    buf[got]=0;
    *out=buf; *out_n=got;
    return 0;
}

static bool utf8_valid(const unsigned char *s,size_t n,size_t *bad) {
    size_t i=0;
    while (i<n) {
        unsigned c=s[i];
        if (c<0x80) { i++; continue; }
        size_t need=0;
        uint32_t cp=0;
        if ((c&0xE0)==0xC0) { need=1; cp=c&0x1F; if (cp<2) goto fail; }
        else if ((c&0xF0)==0xE0) { need=2; cp=c&0x0F; }
        else if ((c&0xF8)==0xF0) { need=3; cp=c&0x07; if (cp>4) goto fail; }
        else goto fail;
        if (i+need>=n) goto fail;
        for (size_t j=1;j<=need;j++) {
            unsigned d=s[i+j];
            if ((d&0xC0)!=0x80) goto fail;
            cp=(cp<<6)|(d&0x3F);
        }
        if ((need==2 && cp<0x800) || (need==3 && cp<0x10000) ||
            cp>0x10FFFF || (cp>=0xD800 && cp<=0xDFFF)) goto fail;
        i+=need+1;
        continue;
fail:
        if (bad) *bad=i;
        return false;
    }
    return true;
}

typedef enum {
    TK_EOF=0, TK_NEWLINE, TK_IDENT, TK_INTLIT, TK_REALLIT, TK_STRING,
    TK_KET0, TK_KET1,
    TK_LET, TK_FN, TK_RETURN, TK_WHEN, TK_ELSE, TK_REPEAT,
    TK_TRUE, TK_FALSE, TK_EMIT, TK_GUARD, TK_OBSERVE, TK_SIMULATE,
    TK_PATCH, TK_QBIT, TK_BIT, TK_H, TK_X, TK_Z, TK_CNOT,
    TK_MEASURE, TK_PROBS, TK_RESET, TK_AND, TK_OR, TK_NOT,
    TK_PLUS, TK_MINUS, TK_STAR, TK_SLASH, TK_PERCENT,
    TK_EQEQ, TK_NEQ, TK_LT, TK_LE, TK_GT, TK_GE,
    TK_ASSIGN, TK_ARROW, TK_COLON, TK_COMMA, TK_DOT,
    TK_LPAREN, TK_RPAREN, TK_LBRACE, TK_RBRACE,
    TK_LBRACKET, TK_RBRACKET, TK_SEMI
} TokenKind;

typedef struct {
    TokenKind kind;
    char *text;
    Span span;
} Token;

typedef struct {
    Token *v;
    size_t n,cap;
} TokenVec;

static const char *token_name(TokenKind k) {
    static const char *names[]={
        "EOF","NEWLINE","IDENT","INT","REAL","STRING","|0>","|1>",
        "let","fn","return","when","else","repeat","true","false","emit",
        "guard","observe","simulate","patch","qbit","bit","h","x","z","cnot",
        "measure","probs","reset","and","or","not","+","-","*","/","%",
        "==","!=","<","<=",">",">=","=","->",":",",",".","(",")","{","}",
        "[","]",";"
    };
    return (size_t)k<ARRAY_LEN(names)?names[k]:"?";
}

static void token_push(TokenVec *a, Token t) {
    if (a->n==a->cap) {
        a->cap=a->cap?a->cap*2:128;
        a->v=xrealloc(a->v,a->cap*sizeof(*a->v));
    }
    a->v[a->n++]=t;
}

static bool ascii_letter(unsigned char c) {
    return (c>='A'&&c<='Z')||(c>='a'&&c<='z')||c=='_';
}
static bool ascii_digit(unsigned char c) { return c>='0'&&c<='9'; }

typedef struct {
    const char *file;
    const unsigned char *s;
    size_t n,i;
    uint32_t line,col;
    int paren_depth;
    const qnova_limits *limits;
    qnova_error *err;
    TokenVec out;
} Lexer;

static Span lx_span(Lexer *l,uint32_t sl,uint32_t sc) {
    Span s={sl,sc,l->line,l->col};
    return s;
}

static int keyword_kind(const char *s) {
    struct K { const char *s; TokenKind k; };
    static const struct K ks[]={
        {"let",TK_LET},{"fn",TK_FN},{"return",TK_RETURN},{"when",TK_WHEN},
        {"else",TK_ELSE},{"repeat",TK_REPEAT},{"true",TK_TRUE},{"false",TK_FALSE},
        {"emit",TK_EMIT},{"guard",TK_GUARD},{"observe",TK_OBSERVE},
        {"simulate",TK_SIMULATE},{"patch",TK_PATCH},{"qbit",TK_QBIT},
        {"bit",TK_BIT},{"h",TK_H},{"x",TK_X},{"z",TK_Z},{"cnot",TK_CNOT},
        {"measure",TK_MEASURE},{"probs",TK_PROBS},{"reset",TK_RESET},
        {"and",TK_AND},{"or",TK_OR},{"not",TK_NOT},
    };
    for (size_t i=0;i<ARRAY_LEN(ks);i++) if (!strcmp(s,ks[i].s)) return ks[i].k;
    return TK_IDENT;
}

static void lx_advance(Lexer *l) {
    if (l->i>=l->n) return;
    unsigned char c=l->s[l->i++];
    if (c=='\n') { l->line++; l->col=1; }
    else l->col++;
}

static int lx_emit(Lexer *l, TokenKind k, char *text, uint32_t sl,uint32_t sc) {
    if (l->out.n>=l->limits->tokens) {
        free(text);
        set_error(l->err,1001,l->file,sl,sc,"invalid source character");
        return -1;
    }
    token_push(&l->out,(Token){k,text,lx_span(l,sl,sc)});
    return 0;
}

static char *decode_string(const unsigned char *p,size_t n,bool *ok) {
    char *o=xmalloc(n+1);
    size_t w=0;
    *ok=true;
    for (size_t i=0;i<n;i++) {
        unsigned char c=p[i];
        if (c!='\\') { o[w++]=(char)c; continue; }
        if (++i>=n) { *ok=false; break; }
        c=p[i];
        switch(c) {
            case 'n': o[w++]='\n'; break;
            case 'r': o[w++]='\r'; break;
            case 't': o[w++]='\t'; break;
            case '"': o[w++]='"'; break;
            case '\\': o[w++]='\\'; break;
            default: *ok=false; break;
        }
        if (!*ok) break;
    }
    o[w]=0;
    return o;
}

static int lex_source(const char *file,const char *src,size_t n,
                      const qnova_limits *limits,TokenVec *out,qnova_error *err) {
    size_t bad=0;
    if (!utf8_valid((const unsigned char*)src,n,&bad)) {
        uint32_t line=1,col=1;
        for (size_t i=0;i<bad;i++) {
            if (src[i]=='\n') { line++; col=1; } else col++;
        }
        set_error(err,1004,file,line,col,"invalid UTF-8 sequence");
        return -1;
    }
    Lexer l={file,(const unsigned char*)src,n,0,1,1,0,limits,err,{0}};
    while (l.i<n && !err->code) {
        unsigned char c=l.s[l.i];
        if (c==' '||c=='\t'||c=='\r') { lx_advance(&l); continue; }
        if (c=='#') {
            while (l.i<n && l.s[l.i]!='\n') lx_advance(&l);
            continue;
        }
        uint32_t sl=l.line,sc=l.col;
        if (c=='\n') {
            lx_advance(&l);
            if (l.paren_depth==0 && lx_emit(&l,TK_NEWLINE,xstrdup("\\n"),sl,sc)) break;
            continue;
        }
        if (ascii_letter(c)) {
            size_t st=l.i;
            while (l.i<n && (ascii_letter(l.s[l.i])||ascii_digit(l.s[l.i]))) lx_advance(&l);
            char *s=xstrndup((const char*)l.s+st,l.i-st);
            int k=keyword_kind(s);
            if (lx_emit(&l,(TokenKind)k,s,sl,sc)) break;
            continue;
        }
        if (ascii_digit(c)) {
            size_t st=l.i;
            while (l.i<n && ascii_digit(l.s[l.i])) lx_advance(&l);
            TokenKind k=TK_INTLIT;
            if (l.i<n && l.s[l.i]=='.') {
                k=TK_REALLIT;
                lx_advance(&l);
                if (l.i>=n || !ascii_digit(l.s[l.i])) {
                    set_error(err,1003,file,sl,sc,"malformed numeric literal");
                    break;
                }
                while (l.i<n && ascii_digit(l.s[l.i])) lx_advance(&l);
            }
            if (l.i<n && l.s[l.i]=='.') {
                set_error(err,1003,file,sl,sc,"malformed numeric literal");
                break;
            }
            if (lx_emit(&l,k,xstrndup((const char*)l.s+st,l.i-st),sl,sc)) break;
            continue;
        }
        if (c=='"') {
            lx_advance(&l);
            size_t st=l.i;
            bool esc=false,closed=false;
            while (l.i<n) {
                unsigned char d=l.s[l.i];
                if (d=='\n') break;
                if (!esc && d=='"') { closed=true; break; }
                if (!esc && d=='\\') { esc=true; lx_advance(&l); continue; }
                esc=false;
                lx_advance(&l);
            }
            if (!closed) {
                set_error(err,1002,file,sl,sc,"unterminated string literal");
                break;
            }
            size_t en=l.i;
            lx_advance(&l);
            bool ok=true;
            char *s=decode_string(l.s+st,en-st,&ok);
            if (!ok) {
                free(s);
                set_error(err,1001,file,sl,sc,"invalid source character");
                break;
            }
            if (lx_emit(&l,TK_STRING,s,sl,sc)) break;
            continue;
        }
        if (c=='|' && l.i+2<n && l.s[l.i+2]=='>' &&
            (l.s[l.i+1]=='0'||l.s[l.i+1]=='1')) {
            TokenKind k=l.s[l.i+1]=='0'?TK_KET0:TK_KET1;
            lx_advance(&l); lx_advance(&l); lx_advance(&l);
            if (lx_emit(&l,k,xstrdup(k==TK_KET0?"|0>":"|1>"),sl,sc)) break;
            continue;
        }
        #define ONE(ch,kind) case ch: lx_advance(&l); if(lx_emit(&l,kind,xstrdup(#ch),sl,sc)){} break
        switch(c) {
            case '+': lx_advance(&l); lx_emit(&l,TK_PLUS,xstrdup("+"),sl,sc); break;
            case '*': lx_advance(&l); lx_emit(&l,TK_STAR,xstrdup("*"),sl,sc); break;
            case '/': lx_advance(&l); lx_emit(&l,TK_SLASH,xstrdup("/"),sl,sc); break;
            case '%': lx_advance(&l); lx_emit(&l,TK_PERCENT,xstrdup("%"),sl,sc); break;
            case ':': lx_advance(&l); lx_emit(&l,TK_COLON,xstrdup(":"),sl,sc); break;
            case ',': lx_advance(&l); lx_emit(&l,TK_COMMA,xstrdup(","),sl,sc); break;
            case '.': lx_advance(&l); lx_emit(&l,TK_DOT,xstrdup("."),sl,sc); break;
            case ';': lx_advance(&l); lx_emit(&l,TK_SEMI,xstrdup(";"),sl,sc); break;
            case '[': lx_advance(&l); lx_emit(&l,TK_LBRACKET,xstrdup("["),sl,sc); break;
            case ']': lx_advance(&l); lx_emit(&l,TK_RBRACKET,xstrdup("]"),sl,sc); break;
            case '(':
                lx_advance(&l); l.paren_depth++;
                lx_emit(&l,TK_LPAREN,xstrdup("("),sl,sc); break;
            case ')':
                lx_advance(&l);
                if (l.paren_depth>0) l.paren_depth--;
                lx_emit(&l,TK_RPAREN,xstrdup(")"),sl,sc); break;
            case '{': lx_advance(&l); lx_emit(&l,TK_LBRACE,xstrdup("{"),sl,sc); break;
            case '}': lx_advance(&l); lx_emit(&l,TK_RBRACE,xstrdup("}"),sl,sc); break;
            case '=':
                lx_advance(&l);
                if (l.i<n && l.s[l.i]=='=') {
                    lx_advance(&l); lx_emit(&l,TK_EQEQ,xstrdup("=="),sl,sc);
                } else lx_emit(&l,TK_ASSIGN,xstrdup("="),sl,sc);
                break;
            case '!':
                lx_advance(&l);
                if (l.i<n && l.s[l.i]=='=') {
                    lx_advance(&l); lx_emit(&l,TK_NEQ,xstrdup("!="),sl,sc);
                } else set_error(err,1001,file,sl,sc,"invalid source character");
                break;
            case '<':
                lx_advance(&l);
                if (l.i<n && l.s[l.i]=='=') {
                    lx_advance(&l); lx_emit(&l,TK_LE,xstrdup("<="),sl,sc);
                } else lx_emit(&l,TK_LT,xstrdup("<"),sl,sc);
                break;
            case '>':
                lx_advance(&l);
                if (l.i<n && l.s[l.i]=='=') {
                    lx_advance(&l); lx_emit(&l,TK_GE,xstrdup(">="),sl,sc);
                } else lx_emit(&l,TK_GT,xstrdup(">"),sl,sc);
                break;
            case '-':
                lx_advance(&l);
                if (l.i<n && l.s[l.i]=='>') {
                    lx_advance(&l); lx_emit(&l,TK_ARROW,xstrdup("->"),sl,sc);
                } else lx_emit(&l,TK_MINUS,xstrdup("-"),sl,sc);
                break;
            default:
                set_error(err,1001,file,sl,sc,"invalid source character");
                break;
        }
        #undef ONE
    }
    if (!err->code) {
        uint32_t sl=l.line,sc=l.col;
        lx_emit(&l,TK_EOF,xstrdup(""),sl,sc);
    }
    if (err->code) {
        for (size_t i=0;i<l.out.n;i++) free(l.out.v[i].text);
        free(l.out.v);
        return -1;
    }
    *out=l.out;
    return 0;
}

static void free_tokens(TokenVec *t) {
    for (size_t i=0;i<t->n;i++) free(t->v[i].text);
    free(t->v); memset(t,0,sizeof(*t));
}


typedef enum {
    TY_UNKNOWN=0, TY_INT, TY_REAL, TY_BOOL, TY_STRING, TY_BIT, TY_QBIT, TY_UNIT, TY_PROBS
} Type;

static const char *type_name(Type t) {
    switch(t) {
        case TY_INT:return "int"; case TY_REAL:return "real"; case TY_BOOL:return "bool";
        case TY_STRING:return "string"; case TY_BIT:return "bit"; case TY_QBIT:return "qbit";
        case TY_UNIT:return "unit"; case TY_PROBS:return "probs"; default:return "?";
    }
}

typedef enum {
    N_PROGRAM, N_BLOCK, N_LET, N_FUNCTION, N_QBIT_DECL,
    N_RETURN, N_EMIT, N_WHEN, N_REPEAT, N_GUARD, N_OBSERVE,
    N_SIMULATE, N_PATCH, N_GATE, N_MEASURE, N_RESET, N_EXPR_STMT,
    N_BINARY, N_UNARY, N_CALL, N_PROBS, N_LITERAL, N_NAME
} NodeKind;

typedef struct Node Node;
typedef struct { Node **v; size_t n,cap; } NodeVec;

typedef struct {
    char *name;
    Type type;
    Span span;
    uint32_t sym_id;
} Param;
typedef struct { Param *v; size_t n,cap; } ParamVec;

struct Node {
    NodeKind kind;
    Span span;
    Type type;
    uint32_t sym_id;
    uint32_t fn_id;
    uint32_t effect;
    union {
        struct { NodeVec items; } list;
        struct { char *name; Type declared; Node *init; } let;
        struct { char *name; ParamVec params; Type ret; Node *body; } fn;
        struct { char *name; int init; } qbit;
        struct { Node *expr; } one;
        struct { Node *cond,*yes,*no; } branch;
        struct { Node *count,*body; } repeat;
        struct { Node *body; } simulate;
        struct { char *name; Node *expr; } patch;
        struct { TokenKind op; char *a,*b; uint32_t a_sym,b_sym; } gate;
        struct { char *q,*target; uint32_t q_sym,target_sym; } measure;
        struct { char *q; uint32_t q_sym; } reset;
        struct { TokenKind op; Node *a,*b; } binary;
        struct { TokenKind op; Node *expr; } unary;
        struct { char *name; NodeVec args; uint32_t target_fn; } call;
        struct { char *q; uint32_t q_sym; } probs;
        struct { int tag; int64_t i; double r; bool b; char *s; } lit;
        struct { char *name; } name;
    } as;
};

static void nodevec_push(NodeVec *a, Node *n) {
    if (a->n==a->cap) { a->cap=a->cap?a->cap*2:16; a->v=xrealloc(a->v,a->cap*sizeof(*a->v)); }
    a->v[a->n++]=n;
}
static void paramvec_push(ParamVec *a,Param p) {
    if (a->n==a->cap) { a->cap=a->cap?a->cap*2:8; a->v=xrealloc(a->v,a->cap*sizeof(*a->v)); }
    a->v[a->n++]=p;
}

typedef struct {
    const char *file;
    TokenVec *tv;
    size_t i;
    size_t node_count;
    const qnova_limits *limits;
    qnova_error *err;
} Parser;

static Token *pk(Parser *p) { return &p->tv->v[p->i]; }
static Token *prev(Parser *p) { return p->i?&p->tv->v[p->i-1]:&p->tv->v[0]; }
static bool is(Parser *p,TokenKind k) { return pk(p)->kind==k; }
static Token *take(Parser *p) { return &p->tv->v[p->i++]; }
static bool match(Parser *p,TokenKind k) { if(is(p,k)){p->i++;return true;}return false; }

static Token *expect(Parser *p,TokenKind k,int code,const char *msg) {
    if (is(p,k)) return take(p);
    Token *t=pk(p);
    set_error(p->err,code,p->file,t->span.line,t->span.col,msg);
    return NULL;
}

static bool identish(TokenKind k) {
    return k==TK_IDENT || k==TK_H || k==TK_X || k==TK_Z;
}
static Token *expect_ident(Parser *p,int code,const char *msg) {
    if (identish(pk(p)->kind)) return take(p);
    Token *t=pk(p);
    set_error(p->err,code,p->file,t->span.line,t->span.col,msg);
    return NULL;
}

static Node *new_node(Parser *p,NodeKind k,Span sp) {
    if (++p->node_count>p->limits->ast_nodes) {
        set_error(p->err,2001,p->file,sp.line,sp.col,"unexpected token");
        return NULL;
    }
    Node *n=xcalloc(1,sizeof(*n));
    n->kind=k; n->span=sp; n->type=TY_UNKNOWN; n->sym_id=U32_NONE; n->fn_id=U32_NONE;
    return n;
}

static Span span_join(Span a,Span b) {
    a.end_line=b.end_line; a.end_col=b.end_col; return a;
}

static void skip_seps(Parser *p) {
    while (is(p,TK_NEWLINE)||is(p,TK_SEMI)) take(p);
}

static bool terminator(Parser *p) {
    if (match(p,TK_SEMI)) { while(match(p,TK_NEWLINE)){} return true; }
    if (match(p,TK_NEWLINE)) { while(match(p,TK_NEWLINE)||match(p,TK_SEMI)){} return true; }
    Token *t=pk(p);
    set_error(p->err,2005,p->file,t->span.line,t->span.col,"invalid statement terminator");
    return false;
}

static Type parse_type(Parser *p) {
    Token *t=pk(p);
    if (match(p,TK_QBIT)) return TY_QBIT;
    if (match(p,TK_BIT)) return TY_BIT;
    if (is(p,TK_IDENT)) {
        if (!strcmp(t->text,"int")) { take(p); return TY_INT; }
        if (!strcmp(t->text,"real")) { take(p); return TY_REAL; }
        if (!strcmp(t->text,"bool")) { take(p); return TY_BOOL; }
        if (!strcmp(t->text,"string")) { take(p); return TY_STRING; }
        if (!strcmp(t->text,"unit")) { take(p); return TY_UNIT; }
    }
    set_error(p->err,2002,p->file,t->span.line,t->span.col,"expected token missing");
    return TY_UNKNOWN;
}

static Node *parse_expr(Parser *p);
static Node *parse_block(Parser *p);
static Node *parse_item(Parser *p);

static Node *parse_primary(Parser *p) {
    Token *t=pk(p);
    if (match(p,TK_INTLIT)) {
        Node *n=new_node(p,N_LITERAL,t->span); if(!n)return NULL;
        errno=0; char *e=NULL; long long v=strtoll(t->text,&e,10);
        if(errno||!e||*e){set_error(p->err,1003,p->file,t->span.line,t->span.col,"malformed numeric literal");return n;}
        n->as.lit.tag=TY_INT; n->as.lit.i=(int64_t)v; return n;
    }
    if (match(p,TK_REALLIT)) {
        Node *n=new_node(p,N_LITERAL,t->span); if(!n)return NULL;
        errno=0; char *e=NULL; double v=strtod(t->text,&e);
        if(errno||!e||*e||!isfinite(v)){set_error(p->err,1003,p->file,t->span.line,t->span.col,"malformed numeric literal");return n;}
        n->as.lit.tag=TY_REAL; n->as.lit.r=v; return n;
    }
    if (match(p,TK_STRING)) {
        Node *n=new_node(p,N_LITERAL,t->span); if(!n)return NULL;
        n->as.lit.tag=TY_STRING; n->as.lit.s=xstrdup(t->text); return n;
    }
    if (match(p,TK_TRUE)||match(p,TK_FALSE)) {
        Token *u=prev(p); Node *n=new_node(p,N_LITERAL,u->span); if(!n)return NULL;
        n->as.lit.tag=TY_BOOL; n->as.lit.b=u->kind==TK_TRUE; return n;
    }
    if (match(p,TK_PROBS)) {
        Token *st=t;
        if (!expect(p,TK_LPAREN,2002,"expected token missing")) return NULL;
        Token *q=expect_ident(p,2002,"expected token missing"); if(!q)return NULL;
        Token *rp=expect(p,TK_RPAREN,2004,"unmatched delimiter"); if(!rp)return NULL;
        Node *n=new_node(p,N_PROBS,span_join(st->span,rp->span)); if(!n)return NULL;
        n->as.probs.q=xstrdup(q->text); n->as.probs.q_sym=U32_NONE; return n;
    }
    if (identish(pk(p)->kind)) {
        Token *name=take(p);
        if (match(p,TK_LPAREN)) {
            Node *n=new_node(p,N_CALL,name->span); if(!n)return NULL;
            n->as.call.name=xstrdup(name->text); n->as.call.target_fn=U32_NONE;
            if (!is(p,TK_RPAREN)) {
                for (;;) {
                    Node *a=parse_expr(p); if(!a)return n;
                    nodevec_push(&n->as.call.args,a);
                    if (!match(p,TK_COMMA)) break;
                }
            }
            Token *rp=expect(p,TK_RPAREN,2004,"unmatched delimiter");
            if(rp)n->span=span_join(n->span,rp->span);
            return n;
        }
        Node *n=new_node(p,N_NAME,name->span); if(!n)return NULL;
        n->as.name.name=xstrdup(name->text); return n;
    }
    if (match(p,TK_LPAREN)) {
        Token *lp=t; Node *e=parse_expr(p);
        Token *rp=expect(p,TK_RPAREN,2004,"unmatched delimiter");
        if(e&&rp)e->span=span_join(lp->span,rp->span);
        return e;
    }
    set_error(p->err,2001,p->file,t->span.line,t->span.col,"unexpected token");
    return NULL;
}

static Node *parse_unary(Parser *p) {
    if (is(p,TK_MINUS)||is(p,TK_NOT)) {
        Token *op=take(p); Node *e=parse_unary(p);
        if(!e)return NULL;
        Node *n=new_node(p,N_UNARY,span_join(op->span,e->span)); if(!n)return NULL;
        n->as.unary.op=op->kind; n->as.unary.expr=e; return n;
    }
    return parse_primary(p);
}

static Node *parse_factor(Parser *p) {
    Node *a=parse_unary(p);
    while (!p->err->code && (is(p,TK_STAR)||is(p,TK_SLASH)||is(p,TK_PERCENT))) {
        Token *op=take(p); Node *b=parse_unary(p);
        Node *n=new_node(p,N_BINARY,span_join(a->span,b->span)); if(!n)return a;
        n->as.binary.op=op->kind;n->as.binary.a=a;n->as.binary.b=b;a=n;
    }
    return a;
}
static Node *parse_term(Parser *p) {
    Node *a=parse_factor(p);
    while (!p->err->code && (is(p,TK_PLUS)||is(p,TK_MINUS))) {
        Token *op=take(p); Node *b=parse_factor(p);
        Node *n=new_node(p,N_BINARY,span_join(a->span,b->span)); if(!n)return a;
        n->as.binary.op=op->kind;n->as.binary.a=a;n->as.binary.b=b;a=n;
    }
    return a;
}
static Node *parse_cmp(Parser *p) {
    Node *a=parse_term(p);
    while (!p->err->code && (is(p,TK_LT)||is(p,TK_LE)||is(p,TK_GT)||is(p,TK_GE))) {
        Token *op=take(p); Node *b=parse_term(p);
        Node *n=new_node(p,N_BINARY,span_join(a->span,b->span)); if(!n)return a;
        n->as.binary.op=op->kind;n->as.binary.a=a;n->as.binary.b=b;a=n;
    }
    return a;
}
static Node *parse_eq(Parser *p) {
    Node *a=parse_cmp(p);
    while (!p->err->code && (is(p,TK_EQEQ)||is(p,TK_NEQ))) {
        Token *op=take(p); Node *b=parse_cmp(p);
        Node *n=new_node(p,N_BINARY,span_join(a->span,b->span)); if(!n)return a;
        n->as.binary.op=op->kind;n->as.binary.a=a;n->as.binary.b=b;a=n;
    }
    return a;
}
static Node *parse_and(Parser *p) {
    Node *a=parse_eq(p);
    while (!p->err->code && match(p,TK_AND)) {
        Token *op=prev(p); Node *b=parse_eq(p);
        Node *n=new_node(p,N_BINARY,span_join(a->span,b->span)); if(!n)return a;
        n->as.binary.op=op->kind;n->as.binary.a=a;n->as.binary.b=b;a=n;
    }
    return a;
}
static Node *parse_or(Parser *p) {
    Node *a=parse_and(p);
    while (!p->err->code && match(p,TK_OR)) {
        Token *op=prev(p); Node *b=parse_and(p);
        Node *n=new_node(p,N_BINARY,span_join(a->span,b->span)); if(!n)return a;
        n->as.binary.op=op->kind;n->as.binary.a=a;n->as.binary.b=b;a=n;
    }
    return a;
}
static Node *parse_expr(Parser *p) { return parse_or(p); }

static Node *parse_block(Parser *p) {
    Token *lb=expect(p,TK_LBRACE,2002,"expected token missing"); if(!lb)return NULL;
    Node *n=new_node(p,N_BLOCK,lb->span); if(!n)return NULL;
    skip_seps(p);
    while (!is(p,TK_RBRACE)&&!is(p,TK_EOF)&&!p->err->code) {
        Node *x=parse_item(p); if(x)nodevec_push(&n->as.list.items,x);
        skip_seps(p);
    }
    Token *rb=expect(p,TK_RBRACE,2004,"unmatched delimiter");
    if(rb)n->span=span_join(n->span,rb->span);
    return n;
}

static Node *parse_let(Parser *p) {
    Token *st=take(p);
    Token *name=expect_ident(p,2002,"expected token missing"); if(!name)return NULL;
    Type declared=TY_UNKNOWN;
    if (match(p,TK_COLON)) declared=parse_type(p);
    if (!expect(p,TK_ASSIGN,2002,"expected token missing")) return NULL;
    Node *init=parse_expr(p); if(!init)return NULL;
    if(!terminator(p))return NULL;
    Node *n=new_node(p,N_LET,span_join(st->span,init->span)); if(!n)return NULL;
    n->as.let.name=xstrdup(name->text); n->as.let.declared=declared;n->as.let.init=init; return n;
}

static Node *parse_fn(Parser *p) {
    Token *st=take(p);
    Token *name=expect_ident(p,2002,"expected token missing"); if(!name)return NULL;
    if(!expect(p,TK_LPAREN,2002,"expected token missing"))return NULL;
    Node *n=new_node(p,N_FUNCTION,st->span); if(!n)return NULL;
    n->as.fn.name=xstrdup(name->text); n->as.fn.ret=TY_UNIT;
    if(!is(p,TK_RPAREN)) {
        for(;;) {
            Token *pn=expect_ident(p,2002,"expected token missing"); if(!pn)return n;
            if(!expect(p,TK_COLON,2002,"expected token missing"))return n;
            Type pt=parse_type(p);
            paramvec_push(&n->as.fn.params,(Param){xstrdup(pn->text),pt,pn->span,U32_NONE});
            if(!match(p,TK_COMMA))break;
        }
    }
    if(!expect(p,TK_RPAREN,2004,"unmatched delimiter"))return n;
    if(match(p,TK_ARROW))n->as.fn.ret=parse_type(p);
    n->as.fn.body=parse_block(p);
    if(n->as.fn.body)n->span=span_join(st->span,n->as.fn.body->span);
    return n;
}

static Node *parse_qbit_decl(Parser *p) {
    Token *st=take(p);
    Token *name=expect_ident(p,2003,"invalid qbit declaration"); if(!name)return NULL;
    if(!expect(p,TK_ASSIGN,2003,"invalid qbit declaration"))return NULL;
    int init=-1; Token *k=pk(p);
    if(match(p,TK_KET0))init=0; else if(match(p,TK_KET1))init=1;
    else {set_error(p->err,2003,p->file,k->span.line,k->span.col,"invalid qbit declaration");return NULL;}
    if(!terminator(p))return NULL;
    Node *n=new_node(p,N_QBIT_DECL,span_join(st->span,prev(p)->span)); if(!n)return NULL;
    n->as.qbit.name=xstrdup(name->text);n->as.qbit.init=init;return n;
}

static Node *parse_item(Parser *p) {
    Token *st=pk(p);
    if(is(p,TK_LET))return parse_let(p);
    if(is(p,TK_FN))return parse_fn(p);
    if(is(p,TK_QBIT))return parse_qbit_decl(p);
    if(is(p,TK_LBRACE))return parse_block(p);
    if(match(p,TK_RETURN)) {
        Node *e=NULL;
        if(!is(p,TK_NEWLINE)&&!is(p,TK_SEMI)) e=parse_expr(p);
        if(!terminator(p))return NULL;
        Node *n=new_node(p,N_RETURN,e?span_join(st->span,e->span):st->span); if(!n)return NULL;
        n->as.one.expr=e;return n;
    }
    if(match(p,TK_EMIT)) {
        Node *e=parse_expr(p); if(!e||!terminator(p))return NULL;
        Node *n=new_node(p,N_EMIT,span_join(st->span,e->span)); if(!n)return NULL;n->as.one.expr=e;return n;
    }
    if(match(p,TK_WHEN)) {
        Node *c=parse_expr(p); Node *yes=parse_block(p); Node *no=NULL;
        if(match(p,TK_ELSE))no=parse_block(p);
        if(!c||!yes)return NULL;
        Node *n=new_node(p,N_WHEN,span_join(st->span,(no?no:yes)->span)); if(!n)return NULL;
        n->as.branch.cond=c;n->as.branch.yes=yes;n->as.branch.no=no;return n;
    }
    if(match(p,TK_REPEAT)) {
        Node *c=parse_expr(p);Node *b=parse_block(p);if(!c||!b)return NULL;
        Node *n=new_node(p,N_REPEAT,span_join(st->span,b->span));if(!n)return NULL;
        n->as.repeat.count=c;n->as.repeat.body=b;return n;
    }
    if(match(p,TK_GUARD)) {
        Node *c=parse_expr(p);Node *yes=parse_block(p);Node *no=NULL;
        if(match(p,TK_ELSE)) no=parse_block(p);
        if(!c||!yes) return NULL;
        Node *n=new_node(p,N_GUARD,span_join(st->span,(no?no:yes)->span));if(!n)return NULL;
        n->as.branch.cond=c;n->as.branch.yes=yes;n->as.branch.no=no;return n;
    }
    if(match(p,TK_OBSERVE)) {
        Node *e=parse_expr(p);if(!e||!terminator(p))return NULL;
        Node *n=new_node(p,N_OBSERVE,span_join(st->span,e->span));if(!n)return NULL;n->as.one.expr=e;return n;
    }
    if(match(p,TK_SIMULATE)) {
        Node *b=parse_block(p);if(!b)return NULL;
        Node *n=new_node(p,N_SIMULATE,span_join(st->span,b->span));if(!n)return NULL;n->as.simulate.body=b;return n;
    }
    if(match(p,TK_PATCH)) {
        Token *name=expect_ident(p,2002,"expected token missing");if(!name)return NULL;
        if(!expect(p,TK_ASSIGN,2002,"expected token missing"))return NULL;
        Node *e=parse_expr(p);if(!e||!terminator(p))return NULL;
        Node *n=new_node(p,N_PATCH,span_join(st->span,e->span));if(!n)return NULL;
        n->as.patch.name=xstrdup(name->text);n->as.patch.expr=e;return n;
    }
    bool gate_start = false;
    if (is(p,TK_CNOT)) gate_start = true;
    else if ((is(p,TK_H)||is(p,TK_X)||is(p,TK_Z)) && p->i+1<p->tv->n &&
             identish(p->tv->v[p->i+1].kind)) gate_start = true;
    if(gate_start) {
        Token *op=take(p);Token *a=expect_ident(p,2002,"expected token missing");if(!a)return NULL;
        Token *b=NULL;if(op->kind==TK_CNOT){if(!expect(p,TK_COMMA,2002,"expected token missing"))return NULL;b=expect_ident(p,2002,"expected token missing");if(!b)return NULL;}
        if(!terminator(p))return NULL;
        Node *n=new_node(p,N_GATE,span_join(st->span,(b?b:a)->span));if(!n)return NULL;
        n->as.gate.op=op->kind;n->as.gate.a=xstrdup(a->text);n->as.gate.b=b?xstrdup(b->text):NULL;
        n->as.gate.a_sym=n->as.gate.b_sym=U32_NONE;return n;
    }
    if(match(p,TK_MEASURE)) {
        Token *q=expect_ident(p,2002,"expected token missing");if(!q)return NULL;
        Token *target=NULL;if(match(p,TK_ARROW)){target=expect_ident(p,2002,"expected token missing");if(!target)return NULL;}
        if(!terminator(p))return NULL;
        Node *n=new_node(p,N_MEASURE,span_join(st->span,(target?target:q)->span));if(!n)return NULL;
        n->as.measure.q=xstrdup(q->text);n->as.measure.target=target?xstrdup(target->text):NULL;
        n->as.measure.q_sym=n->as.measure.target_sym=U32_NONE;return n;
    }
    if(match(p,TK_RESET)) {
        Token *q=expect_ident(p,2002,"expected token missing");if(!q||!terminator(p))return NULL;
        Node *n=new_node(p,N_RESET,span_join(st->span,q->span));if(!n)return NULL;
        n->as.reset.q=xstrdup(q->text);n->as.reset.q_sym=U32_NONE;return n;
    }
    Node *e=parse_expr(p);if(!e)return NULL;
    if(!terminator(p))return NULL;
    Node *n=new_node(p,N_EXPR_STMT,e->span);if(!n)return NULL;n->as.one.expr=e;return n;
}

static Node *parse_program(const char *file,TokenVec *tv,const qnova_limits *limits,qnova_error *err) {
    Parser p={file,tv,0,0,limits,err};
    Span first=tv->n?tv->v[0].span:(Span){1,1,1,1};
    Node *root=new_node(&p,N_PROGRAM,first);if(!root)return NULL;
    skip_seps(&p);
    while(!is(&p,TK_EOF)&&!err->code){
        Node *n=parse_item(&p);if(n)nodevec_push(&root->as.list.items,n);
        skip_seps(&p);
    }
    if(!err->code){
        Token *e=expect(&p,TK_EOF,2001,"unexpected token");
        if(e)root->span=span_join(root->span,e->span);
    }
    return root;
}


typedef enum { SYM_LET, SYM_PARAM, SYM_FN, SYM_QBIT, SYM_BIT_TARGET } SymKind;
typedef struct {
    char *name;
    SymKind kind;
    Type type;
    uint32_t id;
    uint32_t fn_id;
    uint32_t owner_fn;
    Node *fn_node;
    Span span;
} Symbol;

typedef struct Scope Scope;
struct Scope {
    Scope *parent;
    Symbol *v;
    size_t n,cap;
};

enum { EFF_MUTATE=1u<<0, EFF_OUTPUT=1u<<1, EFF_QUANTUM=1u<<2, EFF_OBSERVE=1u<<3 };

typedef struct {
    const char *file;
    const qnova_limits *limits;
    qnova_error *err;
    uint32_t next_sym;
    uint32_t next_fn;
    NodeVec funcs;
    Node *current_fn;
    uint32_t owner_fn;
} Sem;

static Symbol *scope_local(Scope *s,const char *name) {
    for(size_t i=0;i<s->n;i++)if(!strcmp(s->v[i].name,name))return &s->v[i];
    return NULL;
}
static Symbol *scope_find(Scope *s,const char *name) {
    for(;s;s=s->parent){Symbol *x=scope_local(s,name);if(x)return x;}return NULL;
}
static Symbol *scope_add(Scope *s,Symbol x) {
    if(s->n==s->cap){s->cap=s->cap?s->cap*2:16;s->v=xrealloc(s->v,s->cap*sizeof(*s->v));}
    s->v[s->n]=x;return &s->v[s->n++];
}
static void scope_free(Scope *s) {
    for(size_t i=0;i<s->n;i++)free(s->v[i].name);
    free(s->v);
}

static bool assignable(Type dst,Type src) {
    return dst==src || (dst==TY_REAL&&src==TY_INT);
}
static bool numeric(Type t){return t==TY_INT||t==TY_REAL;}
static Type numeric_result(Type a,Type b){return (a==TY_REAL||b==TY_REAL)?TY_REAL:TY_INT;}

static void sem_fail(Sem *s,int code,Span sp,const char *msg){
    set_error(s->err,code,s->file,sp.line,sp.col,msg);
}

static int sem_expr(Sem *s,Scope *sc,Node *n);

static Symbol *require_qbit(Sem *s,Scope *sc,const char *name,Span sp) {
    Symbol *x=scope_find(sc,name);
    if(!x||x->kind!=SYM_QBIT){sem_fail(s,4001,sp,"unknown qbit");return NULL;}
    return x;
}

static int add_decl(Sem *s,Scope *sc,const char *name,SymKind kind,Type type,
                    Span sp,Node *fn_node,uint32_t fn_id,uint32_t *out_id) {
    Symbol *old=scope_local(sc,name);
    if(old){
        sem_fail(s,(kind==SYM_QBIT||old->kind==SYM_QBIT)?4002:3002,sp,
                 (kind==SYM_QBIT||old->kind==SYM_QBIT)?"duplicate qbit declaration":"duplicate declaration");
        return -1;
    }
    for(Scope *p=sc->parent;p;p=p->parent){
        Symbol *o=scope_local(p,name);
        if(o&&o->kind==SYM_QBIT){
            sem_fail(s,kind==SYM_QBIT?4002:3002,sp,
                     kind==SYM_QBIT?"duplicate qbit declaration":"duplicate declaration");
            return -1;
        }
    }
    uint32_t id=++s->next_sym;
    scope_add(sc,(Symbol){xstrdup(name),kind,type,id,fn_id,s->owner_fn,fn_node,sp});
    if(out_id)*out_id=id;
    return 0;
}

static void predeclare_functions(Sem *s,Scope *sc,Node *container) {
    NodeVec *items=&container->as.list.items;
    for(size_t i=0;i<items->n&&!s->err->code;i++){
        Node *n=items->v[i];
        if(n->kind!=N_FUNCTION)continue;
        uint32_t fid=++s->next_fn,sid=U32_NONE;
        if(fid>255){sem_fail(s,3002,n->span,"duplicate declaration");return;}
        if(add_decl(s,sc,n->as.fn.name,SYM_FN,n->as.fn.ret,n->span,n,fid,&sid))return;
        n->fn_id=fid;n->sym_id=sid;
        nodevec_push(&s->funcs,n);
    }
}

static int sem_block_like(Sem *s,Scope *parent,Node *n,bool make_child);

static int sem_expr(Sem *s,Scope *sc,Node *n) {
    if(!n||s->err->code)return -1;
    switch(n->kind){
        case N_LITERAL:
            n->type=(Type)n->as.lit.tag;return 0;
        case N_NAME:{
            Symbol *x=scope_find(sc,n->as.name.name);
            if(!x){sem_fail(s,3001,n->span,"undefined name");return -1;}
            if(x->kind==SYM_FN){sem_fail(s,3003,n->span,"type mismatch");return -1;}
            if(x->kind==SYM_QBIT){sem_fail(s,3005,n->span,"illegal qbit copy");return -1;}
            n->sym_id=x->id;n->type=x->type;return 0;
        }
        case N_PROBS:{
            Symbol *q=require_qbit(s,sc,n->as.probs.q,n->span);if(!q)return -1;
            n->as.probs.q_sym=q->id;n->type=TY_PROBS;return 0;
        }
        case N_UNARY:{
            if(sem_expr(s,sc,n->as.unary.expr))return -1;
            Type t=n->as.unary.expr->type;
            if(n->as.unary.op==TK_MINUS){
                if(!numeric(t)){sem_fail(s,3003,n->span,"type mismatch");return -1;}
                n->type=t;return 0;
            }
            if(t!=TY_BOOL){sem_fail(s,3003,n->span,"type mismatch");return -1;}
            n->type=TY_BOOL;return 0;
        }
        case N_BINARY:{
            if(sem_expr(s,sc,n->as.binary.a)||sem_expr(s,sc,n->as.binary.b))return -1;
            Type a=n->as.binary.a->type,b=n->as.binary.b->type;TokenKind op=n->as.binary.op;
            if(op==TK_AND||op==TK_OR){
                if(a!=TY_BOOL||b!=TY_BOOL){sem_fail(s,3003,n->span,"type mismatch");return -1;}
                n->type=TY_BOOL;return 0;
            }
            if(op==TK_PLUS||op==TK_MINUS||op==TK_STAR||op==TK_SLASH||op==TK_PERCENT){
                if(!numeric(a)||!numeric(b)||(op==TK_PERCENT&&(a!=TY_INT||b!=TY_INT))){
                    sem_fail(s,3003,n->span,"type mismatch");return -1;
                }
                n->type=numeric_result(a,b);return 0;
            }
            if(op==TK_EQEQ||op==TK_NEQ){
                bool ok=(a==b&&(a==TY_INT||a==TY_REAL||a==TY_BOOL||a==TY_STRING||a==TY_BIT)) ||
                        (numeric(a)&&numeric(b));
                if(!ok){sem_fail(s,3003,n->span,"type mismatch");return -1;}
                n->type=TY_BOOL;return 0;
            }
            if(op==TK_LT||op==TK_LE||op==TK_GT||op==TK_GE){
                if(!numeric(a)||!numeric(b)){sem_fail(s,3003,n->span,"type mismatch");return -1;}
                n->type=TY_BOOL;return 0;
            }
            sem_fail(s,3003,n->span,"type mismatch");return -1;
        }
        case N_CALL:{
            Symbol *fn=scope_find(sc,n->as.call.name);
            if(!fn||fn->kind!=SYM_FN){sem_fail(s,3001,n->span,"undefined name");return -1;}
            Node *f=fn->fn_node;
            if(n->as.call.args.n!=f->as.fn.params.n){sem_fail(s,3003,n->span,"type mismatch");return -1;}
            for(size_t i=0;i<n->as.call.args.n;i++){
                if(sem_expr(s,sc,n->as.call.args.v[i]))return -1;
                if(!assignable(f->as.fn.params.v[i].type,n->as.call.args.v[i]->type)){
                    sem_fail(s,3003,n->as.call.args.v[i]->span,"type mismatch");return -1;
                }
            }
            n->as.call.target_fn=fn->fn_id;n->type=f->as.fn.ret;return 0;
        }
        default: sem_fail(s,3003,n->span,"type mismatch");return -1;
    }
}

static bool guarantees_return(Node *n){
    if(!n)return false;
    if(n->kind==N_RETURN)return true;
    if(n->kind==N_PROGRAM||n->kind==N_BLOCK){
        for(size_t i=0;i<n->as.list.items.n;i++)if(guarantees_return(n->as.list.items.v[i]))return true;
        return false;
    }
    if((n->kind==N_WHEN||n->kind==N_GUARD)&&n->as.branch.no)
        return guarantees_return(n->as.branch.yes)&&guarantees_return(n->as.branch.no);
    if(n->kind==N_SIMULATE)return guarantees_return(n->as.simulate.body);
    return false;
}

static int sem_stmt(Sem *s,Scope *sc,Node *n){
    if(!n||s->err->code)return -1;
    switch(n->kind){
        case N_BLOCK:return sem_block_like(s,sc,n,true);
        case N_LET:{
            if(sem_expr(s,sc,n->as.let.init))return -1;
            Type t=n->as.let.declared==TY_UNKNOWN?n->as.let.init->type:n->as.let.declared;
            if(t==TY_QBIT||t==TY_PROBS||!assignable(t,n->as.let.init->type)){
                sem_fail(s,3003,n->span,"type mismatch");return -1;
            }
            n->type=t;
            return add_decl(s,sc,n->as.let.name,SYM_LET,t,n->span,NULL,0,&n->sym_id);
        }
        case N_QBIT_DECL:{
            n->type=TY_QBIT;
            return add_decl(s,sc,n->as.qbit.name,SYM_QBIT,TY_QBIT,n->span,NULL,0,&n->sym_id);
        }
        case N_FUNCTION:{
            Symbol *fs=scope_local(sc,n->as.fn.name);
            if(!fs||fs->fn_node!=n){sem_fail(s,3001,n->span,"undefined name");return -1;}
            Scope fnsc={.parent=sc};
            Node *save_fn=s->current_fn;uint32_t save_owner=s->owner_fn;
            s->current_fn=n;s->owner_fn=n->fn_id;
            for(size_t i=0;i<n->as.fn.params.n&&!s->err->code;i++){
                Param *p=&n->as.fn.params.v[i];
                if(p->type==TY_UNIT){sem_fail(s,3003,p->span,"type mismatch");break;}
                add_decl(s,&fnsc,p->name,SYM_PARAM,p->type,p->span,NULL,0,&p->sym_id);
            }
            if(!s->err->code)sem_block_like(s,&fnsc,n->as.fn.body,true);
            if(!s->err->code&&n->as.fn.ret!=TY_UNIT&&!guarantees_return(n->as.fn.body))
                sem_fail(s,3007,n->span,"return type mismatch");
            scope_free(&fnsc);s->current_fn=save_fn;s->owner_fn=save_owner;
            return s->err->code?-1:0;
        }
        case N_RETURN:{
            if(!s->current_fn){
                if(n->as.one.expr)sem_expr(s,sc,n->as.one.expr);
                n->type=n->as.one.expr?n->as.one.expr->type:TY_UNIT;
                return s->err->code?-1:0;
            }
            Type expected=s->current_fn->as.fn.ret;
            if(n->as.one.expr){
                if(sem_expr(s,sc,n->as.one.expr))return -1;
                if(n->as.one.expr->type==TY_QBIT||!assignable(expected,n->as.one.expr->type)){
                    sem_fail(s,3007,n->span,"return type mismatch");return -1;
                }
            } else if(expected!=TY_UNIT){sem_fail(s,3007,n->span,"return type mismatch");return -1;}
            n->type=expected;return 0;
        }
        case N_EMIT:
        case N_OBSERVE:
        case N_EXPR_STMT:
            return sem_expr(s,sc,n->as.one.expr);
        case N_WHEN:
        case N_GUARD:{
            if(sem_expr(s,sc,n->as.branch.cond))return -1;
            if(n->as.branch.cond->type!=TY_BOOL){sem_fail(s,3004,n->as.branch.cond->span,"condition is not Boolean");return -1;}
            if(sem_block_like(s,sc,n->as.branch.yes,true))return -1;
            if(n->as.branch.no&&sem_block_like(s,sc,n->as.branch.no,true))return -1;
            return 0;
        }
        case N_REPEAT:
            if(sem_expr(s,sc,n->as.repeat.count))return -1;
            if(n->as.repeat.count->type!=TY_INT){sem_fail(s,3003,n->as.repeat.count->span,"type mismatch");return -1;}
            return sem_block_like(s,sc,n->as.repeat.body,true);
        case N_SIMULATE:return sem_block_like(s,sc,n->as.simulate.body,true);
        case N_PATCH:{
            Symbol *x=scope_find(sc,n->as.patch.name);
            if(!x){sem_fail(s,3001,n->span,"undefined name");return -1;}
            if(x->kind!=SYM_LET&&x->kind!=SYM_BIT_TARGET){sem_fail(s,3006,n->span,"invalid patch target");return -1;}
            if(sem_expr(s,sc,n->as.patch.expr))return -1;
            if(!assignable(x->type,n->as.patch.expr->type)){sem_fail(s,3003,n->span,"type mismatch");return -1;}
            n->sym_id=x->id;n->type=x->type;return 0;
        }
        case N_GATE:{
            Symbol *a=require_qbit(s,sc,n->as.gate.a,n->span);if(!a)return -1;
            n->as.gate.a_sym=a->id;
            if(n->as.gate.op==TK_CNOT){
                Symbol *b=require_qbit(s,sc,n->as.gate.b,n->span);if(!b)return -1;
                if(a->id==b->id){sem_fail(s,4003,n->span,"CNOT control equals target");return -1;}
                n->as.gate.b_sym=b->id;
            }
            n->type=TY_UNIT;return 0;
        }
        case N_MEASURE:{
            Symbol *q=require_qbit(s,sc,n->as.measure.q,n->span);if(!q)return -1;
            n->as.measure.q_sym=q->id;
            if(n->as.measure.target){
                Symbol *t=scope_find(sc,n->as.measure.target);
                if(!t){
                    uint32_t id=U32_NONE;
                    if(add_decl(s,sc,n->as.measure.target,SYM_BIT_TARGET,TY_BIT,n->span,NULL,0,&id))return -1;
                    n->as.measure.target_sym=id;
                }else{
                    if(t->type!=TY_BIT||t->kind==SYM_QBIT||t->kind==SYM_FN){
                        sem_fail(s,4005,n->span,"invalid measurement target");return -1;
                    }
                    n->as.measure.target_sym=t->id;
                }
            }
            n->type=TY_BIT;return 0;
        }
        case N_RESET:{
            Symbol *q=require_qbit(s,sc,n->as.reset.q,n->span);if(!q)return -1;
            n->as.reset.q_sym=q->id;n->type=TY_UNIT;return 0;
        }
        default:return sem_expr(s,sc,n);
    }
}

static int sem_block_like(Sem *s,Scope *parent,Node *n,bool make_child){
    Scope local={.parent=parent};Scope *sc=make_child?&local:parent;
    predeclare_functions(s,sc,n);
    for(size_t i=0;i<n->as.list.items.n&&!s->err->code;i++)sem_stmt(s,sc,n->as.list.items.v[i]);
    if(make_child)scope_free(&local);
    return s->err->code?-1:0;
}

static Node *find_fn_by_id(Sem *s,uint32_t id){
    for(size_t i=0;i<s->funcs.n;i++)if(s->funcs.v[i]->fn_id==id)return s->funcs.v[i];
    return NULL;
}

static uint32_t effect_expr(Sem *s,Node *n){
    if(!n)return 0;
    switch(n->kind){
        case N_CALL:{Node *f=find_fn_by_id(s,n->as.call.target_fn);return f?f->effect:0;}
        case N_BINARY:return effect_expr(s,n->as.binary.a)|effect_expr(s,n->as.binary.b);
        case N_UNARY:return effect_expr(s,n->as.unary.expr);
        default:return 0;
    }
}
static uint32_t effect_node(Sem *s,Node *n){
    if(!n)return 0;
    uint32_t e=0;
    switch(n->kind){
        case N_PROGRAM:case N_BLOCK:
            for(size_t i=0;i<n->as.list.items.n;i++)if(n->as.list.items.v[i]->kind!=N_FUNCTION)e|=effect_node(s,n->as.list.items.v[i]);
            break;
        case N_LET:e=effect_expr(s,n->as.let.init);break;
        case N_RETURN:case N_EXPR_STMT:e=effect_expr(s,n->as.one.expr);break;
        case N_EMIT:e=EFF_OUTPUT|effect_expr(s,n->as.one.expr);break;
        case N_OBSERVE:e=EFF_OBSERVE|effect_expr(s,n->as.one.expr);break;
        case N_WHEN:case N_GUARD:
            e=effect_expr(s,n->as.branch.cond)|effect_node(s,n->as.branch.yes)|effect_node(s,n->as.branch.no);break;
        case N_REPEAT:e=effect_expr(s,n->as.repeat.count)|effect_node(s,n->as.repeat.body);break;
        case N_SIMULATE:e=effect_node(s,n->as.simulate.body);break;
        case N_PATCH:e=EFF_MUTATE|effect_expr(s,n->as.patch.expr);break;
        case N_GATE:e=EFF_QUANTUM;break;
        case N_MEASURE:e=EFF_QUANTUM|(n->as.measure.target?EFF_MUTATE:0);break;
        case N_RESET:e=EFF_QUANTUM;break;
        case N_CALL:case N_BINARY:case N_UNARY:e=effect_expr(s,n);break;
        default:break;
    }
    n->effect=e;return e;
}

static int validate_effects(Sem *s,Node *n){
    if(!n||s->err->code)return -1;
    if(n->kind==N_GUARD){
        uint32_t e=effect_expr(s,n->as.branch.cond);
        if(e){sem_fail(s,3008,n->as.branch.cond->span,"invalid effect in context");return -1;}
    }
    if(n->kind==N_OBSERVE){
        uint32_t e=effect_expr(s,n->as.one.expr);
        if(e&(EFF_MUTATE|EFF_QUANTUM)){sem_fail(s,7003,n->as.one.expr->span,"mutation attempted by observe");return -1;}
        if(e){sem_fail(s,3008,n->as.one.expr->span,"invalid effect in context");return -1;}
    }
    switch(n->kind){
        case N_PROGRAM:case N_BLOCK:
            for(size_t i=0;i<n->as.list.items.n;i++) {
                if(validate_effects(s,n->as.list.items.v[i])) return -1;
            }
            break;
        case N_FUNCTION:return validate_effects(s,n->as.fn.body);
        case N_WHEN:case N_GUARD:
            if(validate_effects(s,n->as.branch.yes))return -1;
            if(n->as.branch.no&&validate_effects(s,n->as.branch.no)) return -1;
            break;
        case N_REPEAT:return validate_effects(s,n->as.repeat.body);
        case N_SIMULATE:return validate_effects(s,n->as.simulate.body);
        default:break;
    }
    return 0;
}

static int semantic_analyze(const char *file,Node *root,const qnova_limits *limits,
                            Sem *out,qnova_error *err){
    Sem s={.file=file,.limits=limits,.err=err};
    Scope global={0};
    if(sem_block_like(&s,&global,root,false)){scope_free(&global);*out=s;return -1;}
    for(int pass=0;pass<64;pass++){
        bool changed=false;
        for(size_t i=0;i<s.funcs.n;i++){
            Node *fn=s.funcs.v[i];uint32_t old=fn->effect;
            uint32_t now=effect_node(&s,fn->as.fn.body);
            if(now!=old){fn->effect=now;changed=true;}
        }
        if(!changed)break;
    }
    effect_node(&s,root);
    validate_effects(&s,root);
    scope_free(&global);
    *out=s;
    return err->code?-1:0;
}


enum {
    OP_NOP=0x0000, OP_CONST=0x0001, OP_LOAD=0x0002, OP_STORE=0x0003,
    OP_ADD=0x0004, OP_SUB=0x0005, OP_MUL=0x0006, OP_DIV=0x0007, OP_MOD=0x0008,
    OP_CMP_EQ=0x0010, OP_CMP_NE=0x0011, OP_CMP_LT=0x0012, OP_CMP_LE=0x0013,
    OP_CMP_GT=0x0014, OP_CMP_GE=0x0015, OP_JUMP=0x0020, OP_JUMP_IF_FALSE=0x0021,
    OP_CALL=0x0030, OP_RETURN=0x0031, OP_EMIT=0x0040,
    OP_GUARD_BEGIN=0x0050, OP_GUARD_END=0x0051, OP_OBSERVE=0x0052,
    OP_SIMULATE_BEGIN=0x0053, OP_SIMULATE_END=0x0054, OP_PATCH=0x0055,
    OP_QALLOC=0x0100, OP_QH=0x0101, OP_QX=0x0102, OP_QZ=0x0103,
    OP_QCNOT=0x0104, OP_QPROBS=0x0105, OP_QMEASURE=0x0106, OP_QRESET=0x0107
};

static const char *opcode_name(uint16_t op){
    switch(op){
        case OP_NOP:return"NOP";case OP_CONST:return"CONST";case OP_LOAD:return"LOAD";
        case OP_STORE:return"STORE";case OP_ADD:return"ADD";case OP_SUB:return"SUB";
        case OP_MUL:return"MUL";case OP_DIV:return"DIV";case OP_MOD:return"MOD";
        case OP_CMP_EQ:return"CMP_EQ";case OP_CMP_NE:return"CMP_NE";
        case OP_CMP_LT:return"CMP_LT";case OP_CMP_LE:return"CMP_LE";
        case OP_CMP_GT:return"CMP_GT";case OP_CMP_GE:return"CMP_GE";
        case OP_JUMP:return"JUMP";case OP_JUMP_IF_FALSE:return"JUMP_IF_FALSE";
        case OP_CALL:return"CALL";case OP_RETURN:return"RETURN";case OP_EMIT:return"EMIT";
        case OP_GUARD_BEGIN:return"GUARD_BEGIN";case OP_GUARD_END:return"GUARD_END";
        case OP_OBSERVE:return"OBSERVE";case OP_SIMULATE_BEGIN:return"SIMULATE_BEGIN";
        case OP_SIMULATE_END:return"SIMULATE_END";case OP_PATCH:return"PATCH";
        case OP_QALLOC:return"QALLOC";case OP_QH:return"QH";case OP_QX:return"QX";
        case OP_QZ:return"QZ";case OP_QCNOT:return"QCNOT";case OP_QPROBS:return"QPROBS";
        case OP_QMEASURE:return"QMEASURE";case OP_QRESET:return"QRESET";default:return"?";
    }
}

typedef struct {
    uint8_t tag,flags;
    uint32_t len;
    unsigned char *data;
} ConstEnt;
typedef struct { ConstEnt *v; size_t n,cap; } ConstVec;

typedef struct {
    uint16_t op;
    uint8_t flags,nops;
    uint32_t *ops;
    Span span;
    uint32_t effect;
    uint32_t block;
} QIns;
typedef struct { QIns *v; size_t n,cap; } QIR;

typedef struct {
    ConstVec consts;
    QIR code;
    uint32_t entry;
} QbcProg;

static void const_push(ConstVec *v,ConstEnt e){
    if(v->n==v->cap){v->cap=v->cap?v->cap*2:32;v->v=xrealloc(v->v,v->cap*sizeof(*v->v));}
    v->v[v->n++]=e;
}
static uint32_t const_add(ConstVec *v,uint8_t tag,uint8_t flags,const void *data,uint32_t len){
    for(size_t i=0;i<v->n;i++){
        ConstEnt *e=&v->v[i];
        if(e->tag==tag&&e->flags==flags&&e->len==len&&(!len||!memcmp(e->data,data,len)))return (uint32_t)i;
    }
    unsigned char *p=xmalloc(len?len:1);if(len)memcpy(p,data,len);
    const_push(v,(ConstEnt){tag,flags,len,p});
    return (uint32_t)(v->n-1);
}
static uint32_t const_int(ConstVec *v,int64_t x,Type t){
    unsigned char b[8];uint64_t u=(uint64_t)x;
    for(int i=0;i<8;i++)b[i]=(unsigned char)(u>>(8*i));
    return const_add(v,0x01,(uint8_t)t,b,8);
}
static uint32_t const_real(ConstVec *v,double d){
    uint64_t u=0;memcpy(&u,&d,8);unsigned char b[8];
    for(int i=0;i<8;i++)b[i]=(unsigned char)(u>>(8*i));
    return const_add(v,0x02,(uint8_t)TY_REAL,b,8);
}
static uint32_t const_string(ConstVec *v,const char *s){
    return const_add(v,0x03,(uint8_t)TY_STRING,s,(uint32_t)strlen(s));
}
static void qir_push(QIR *q,QIns in){
    if(q->n==q->cap){q->cap=q->cap?q->cap*2:128;q->v=xrealloc(q->v,q->cap*sizeof(*q->v));}
    q->v[q->n++]=in;
}

typedef struct { size_t ins; uint32_t fnid; } CallPatch;
typedef struct { CallPatch *v; size_t n,cap; } CallPatchVec;
static void callpatch_push(CallPatchVec *v,CallPatch x){
    if(v->n==v->cap){v->cap=v->cap?v->cap*2:32;v->v=xrealloc(v->v,v->cap*sizeof(*v->v));}
    v->v[v->n++]=x;
}

typedef struct {
    const char *file;
    const qnova_limits *limits;
    qnova_error *err;
    Sem *sem;
    QbcProg p;
    uint32_t owner;
    uint32_t next_reg[256];
    uint32_t fn_entry[256];
    uint32_t block;
    int sim_depth;
    CallPatchVec calls;
} Compiler;

static uint32_t reg_new(Compiler *c){
    uint32_t idx=c->next_reg[c->owner]++;
    if(idx>=0x00ffffffu){set_error(c->err,6004,c->file,1,1,"instruction limit exceeded");return 0;}
    return (c->owner<<24)|idx;
}
static size_t emit_q(Compiler *c,uint16_t op,uint8_t flags,Span sp,uint32_t eff,
                     size_t nops,...){
    if(c->p.code.n>=c->limits->qbc_instructions){
        set_error(c->err,6004,c->file,sp.line,sp.col,"instruction limit exceeded");return 0;
    }
    uint32_t *ops=xcalloc(nops?nops:1,sizeof(uint32_t));
    va_list ap;va_start(ap,nops);
    for(size_t i=0;i<nops;i++)ops[i]=va_arg(ap,uint32_t);
    va_end(ap);
    QIns in={op,flags,(uint8_t)nops,ops,sp,eff,c->block};
    qir_push(&c->p.code,in);
    return c->p.code.n-1;
}
static uint32_t literal_reg(Compiler *c,Type t,int64_t i,double r,const char *s,Span sp){
    uint32_t ci=0;
    if(t==TY_REAL)ci=const_real(&c->p.consts,r);
    else if(t==TY_STRING)ci=const_string(&c->p.consts,s?s:"");
    else ci=const_int(&c->p.consts,i,t);
    uint32_t reg=reg_new(c);
    emit_q(c,OP_CONST,(uint8_t)t,sp,0,2,reg,ci);
    return reg;
}
static uint32_t compile_expr(Compiler *c,Node *n);
static void compile_stmt(Compiler *c,Node *n);

static uint16_t cmp_opcode(TokenKind k){
    switch(k){case TK_EQEQ:return OP_CMP_EQ;case TK_NEQ:return OP_CMP_NE;
        case TK_LT:return OP_CMP_LT;case TK_LE:return OP_CMP_LE;
        case TK_GT:return OP_CMP_GT;case TK_GE:return OP_CMP_GE;default:return 0;}
}

static uint32_t compile_expr(Compiler *c,Node *n){
    if(c->err->code||!n)return 0;
    switch(n->kind){
        case N_LITERAL:
            if(n->type==TY_INT)return literal_reg(c,TY_INT,n->as.lit.i,0,NULL,n->span);
            if(n->type==TY_REAL)return literal_reg(c,TY_REAL,0,n->as.lit.r,NULL,n->span);
            if(n->type==TY_STRING)return literal_reg(c,TY_STRING,0,0,n->as.lit.s,n->span);
            if(n->type==TY_BOOL)return literal_reg(c,TY_BOOL,n->as.lit.b?1:0,0,NULL,n->span);
            return 0;
        case N_NAME:{
            uint32_t r=reg_new(c);emit_q(c,OP_LOAD,(uint8_t)n->type,n->span,0,2,r,n->sym_id);return r;
        }
        case N_PROBS:{
            uint32_t r=reg_new(c);emit_q(c,OP_QPROBS,(uint8_t)TY_PROBS,n->span,0,2,r,n->as.probs.q_sym);return r;
        }
        case N_UNARY:{
            uint32_t a=compile_expr(c,n->as.unary.expr),d=reg_new(c);
            if(n->as.unary.op==TK_MINUS){
                uint32_t z=n->type==TY_REAL?literal_reg(c,TY_REAL,0,0.0,NULL,n->span):literal_reg(c,TY_INT,0,0,NULL,n->span);
                emit_q(c,OP_SUB,(uint8_t)n->type,n->span,n->effect,3,d,z,a);
            }else{
                uint32_t f=literal_reg(c,TY_BOOL,0,0,NULL,n->span);
                emit_q(c,OP_CMP_EQ,(uint8_t)TY_BOOL,n->span,n->effect,3,d,a,f);
            }
            return d;
        }
        case N_BINARY:{
            TokenKind op=n->as.binary.op;
            if(op==TK_AND){
                uint32_t d=literal_reg(c,TY_BOOL,0,0,NULL,n->span);
                uint32_t a=compile_expr(c,n->as.binary.a);
                size_t jf=emit_q(c,OP_JUMP_IF_FALSE,0,n->span,n->effect,2,a,0u);
                uint32_t b=compile_expr(c,n->as.binary.b);
                uint32_t tr=literal_reg(c,TY_BOOL,1,0,NULL,n->span);
                emit_q(c,OP_CMP_EQ,(uint8_t)TY_BOOL,n->span,n->effect,3,d,b,tr);
                c->p.code.v[jf].ops[1]=(uint32_t)c->p.code.n;c->block++;return d;
            }
            if(op==TK_OR){
                uint32_t d=literal_reg(c,TY_BOOL,1,0,NULL,n->span);
                uint32_t a=compile_expr(c,n->as.binary.a);
                size_t jf=emit_q(c,OP_JUMP_IF_FALSE,0,n->span,n->effect,2,a,0u);
                size_t je=emit_q(c,OP_JUMP,0,n->span,n->effect,1,0u);
                c->p.code.v[jf].ops[1]=(uint32_t)c->p.code.n;c->block++;
                uint32_t b=compile_expr(c,n->as.binary.b);
                uint32_t tr=literal_reg(c,TY_BOOL,1,0,NULL,n->span);
                emit_q(c,OP_CMP_EQ,(uint8_t)TY_BOOL,n->span,n->effect,3,d,b,tr);
                c->p.code.v[je].ops[0]=(uint32_t)c->p.code.n;c->block++;return d;
            }
            uint32_t a=compile_expr(c,n->as.binary.a);
            uint32_t b=compile_expr(c,n->as.binary.b);
            uint32_t d=reg_new(c);uint16_t oc=cmp_opcode(op);
            if(!oc){
                if(op==TK_PLUS)oc=OP_ADD;else if(op==TK_MINUS)oc=OP_SUB;
                else if(op==TK_STAR)oc=OP_MUL;else if(op==TK_SLASH)oc=OP_DIV;else oc=OP_MOD;
            }
            emit_q(c,oc,(uint8_t)n->type,n->span,n->effect,3,d,a,b);return d;
        }
        case N_CALL:{
            size_t na=n->as.call.args.n;
            if(na>80){set_error(c->err,6004,c->file,n->span.line,n->span.col,"instruction limit exceeded");return 0;}
            Node *fn=find_fn_by_id(c->sem,n->as.call.target_fn);
            uint32_t d=reg_new(c);
            size_t nop=5+3*na;uint32_t *ops=xcalloc(nop,sizeof(uint32_t));
            ops[0]=d;ops[1]=n->as.call.target_fn;ops[2]=0;ops[3]=(uint32_t)n->type;ops[4]=(uint32_t)na;
            for(size_t i=0;i<na;i++){
                ops[5+3*i]=fn->as.fn.params.v[i].sym_id;
                ops[5+3*i+1]=(uint32_t)fn->as.fn.params.v[i].type;
                ops[5+3*i+2]=compile_expr(c,n->as.call.args.v[i]);
            }
            if(c->p.code.n>=c->limits->qbc_instructions){free(ops);set_error(c->err,6004,c->file,n->span.line,n->span.col,"instruction limit exceeded");return d;}
            QIns in={OP_CALL,(uint8_t)n->type,(uint8_t)nop,ops,n->span,n->effect,c->block};
            qir_push(&c->p.code,in);callpatch_push(&c->calls,(CallPatch){c->p.code.n-1,n->as.call.target_fn});
            return d;
        }
        default:return 0;
    }
}

static void compile_block_items(Compiler *c,Node *b){
    for(size_t i=0;i<b->as.list.items.n&&!c->err->code;i++){
        Node *x=b->as.list.items.v[i];
        if(x->kind!=N_FUNCTION)compile_stmt(c,x);
    }
}
static void compile_stmt(Compiler *c,Node *n){
    if(c->err->code||!n)return;
    switch(n->kind){
        case N_BLOCK:compile_block_items(c,n);break;
        case N_LET:{
            uint32_t r=compile_expr(c,n->as.let.init);
            emit_q(c,OP_STORE,(uint8_t)n->type,n->span,n->effect,2,n->sym_id,r);break;
        }
        case N_QBIT_DECL:emit_q(c,OP_QALLOC,0,n->span,EFF_QUANTUM,2,n->sym_id,(uint32_t)n->as.qbit.init);break;
        case N_RETURN:{
            uint32_t r=0;bool has=n->as.one.expr!=NULL;
            if(has)r=compile_expr(c,n->as.one.expr);
            for(int i=0;i<c->sim_depth;i++)emit_q(c,OP_SIMULATE_END,0,n->span,0,0);
            if(has)emit_q(c,OP_RETURN,(uint8_t)n->type,n->span,n->effect,1,r);
            else emit_q(c,OP_RETURN,(uint8_t)TY_UNIT,n->span,n->effect,0);
            break;
        }
        case N_EMIT:{uint32_t r=compile_expr(c,n->as.one.expr);emit_q(c,OP_EMIT,(uint8_t)n->as.one.expr->type,n->span,n->effect,1,r);break;}
        case N_OBSERVE:{uint32_t r=compile_expr(c,n->as.one.expr);emit_q(c,OP_OBSERVE,(uint8_t)n->as.one.expr->type,n->span,n->effect,1,r);break;}
        case N_EXPR_STMT:(void)compile_expr(c,n->as.one.expr);break;
        case N_WHEN:{
            uint32_t cond=compile_expr(c,n->as.branch.cond);
            size_t jf=emit_q(c,OP_JUMP_IF_FALSE,0,n->span,0,2,cond,0u);
            c->block++;compile_block_items(c,n->as.branch.yes);
            if(n->as.branch.no){
                size_t je=emit_q(c,OP_JUMP,0,n->span,0,1,0u);
                c->p.code.v[jf].ops[1]=(uint32_t)c->p.code.n;c->block++;
                compile_block_items(c,n->as.branch.no);
                c->p.code.v[je].ops[0]=(uint32_t)c->p.code.n;c->block++;
            }else{c->p.code.v[jf].ops[1]=(uint32_t)c->p.code.n;c->block++;}
            break;
        }
        case N_REPEAT:{
            uint32_t count=compile_expr(c,n->as.repeat.count);
            size_t loop=c->p.code.n;
            size_t jf=emit_q(c,OP_JUMP_IF_FALSE,0x80,n->span,0,2,count,0u);
            c->block++;compile_block_items(c,n->as.repeat.body);
            uint32_t one=literal_reg(c,TY_INT,1,0,NULL,n->span);
            emit_q(c,OP_SUB,(uint8_t)TY_INT,n->span,0,3,count,count,one);
            emit_q(c,OP_JUMP,0,n->span,0,1,(uint32_t)loop);
            c->p.code.v[jf].ops[1]=(uint32_t)c->p.code.n;c->block++;
            break;
        }
        case N_GUARD:{
            uint32_t cond=compile_expr(c,n->as.branch.cond);
            size_t gb=emit_q(c,OP_GUARD_BEGIN,0,n->span,0,3,cond,0u,n->as.branch.no?1u:0u);
            c->block++;compile_block_items(c,n->as.branch.yes);
            if(n->as.branch.no){
                size_t je=emit_q(c,OP_JUMP,0,n->span,0,1,0u);
                c->p.code.v[gb].ops[1]=(uint32_t)c->p.code.n;c->block++;
                compile_block_items(c,n->as.branch.no);
                c->p.code.v[je].ops[0]=(uint32_t)c->p.code.n;c->block++;
            }else c->p.code.v[gb].ops[1]=(uint32_t)c->p.code.n;
            emit_q(c,OP_GUARD_END,0,n->span,0,0);break;
        }
        case N_SIMULATE:
            emit_q(c,OP_SIMULATE_BEGIN,0,n->span,0,0);c->sim_depth++;
            compile_block_items(c,n->as.simulate.body);c->sim_depth--;
            emit_q(c,OP_SIMULATE_END,0,n->span,0,0);break;
        case N_PATCH:{
            uint32_t r=compile_expr(c,n->as.patch.expr);
            emit_q(c,OP_PATCH,(uint8_t)n->type,n->span,n->effect,2,n->sym_id,r);break;
        }
        case N_GATE:{
            uint16_t op=n->as.gate.op==TK_H?OP_QH:n->as.gate.op==TK_X?OP_QX:n->as.gate.op==TK_Z?OP_QZ:OP_QCNOT;
            if(op==OP_QCNOT) emit_q(c,op,0,n->span,EFF_QUANTUM,2,n->as.gate.a_sym,n->as.gate.b_sym);
            else emit_q(c,op,0,n->span,EFF_QUANTUM,1,n->as.gate.a_sym);
            break;
        }
        case N_MEASURE:{
            uint32_t d=reg_new(c);
            emit_q(c,OP_QMEASURE,(uint8_t)TY_BIT,n->span,n->effect,3,d,n->as.measure.q_sym,
                   n->as.measure.target?n->as.measure.target_sym:U32_NONE);break;
        }
        case N_RESET:emit_q(c,OP_QRESET,0,n->span,EFF_QUANTUM,1,n->as.reset.q_sym);break;
        case N_FUNCTION:break;
        default:(void)compile_expr(c,n);break;
    }
}

static int compile_program(const char *file,Node *root,Sem *sem,const qnova_limits *limits,
                           QbcProg *out,qnova_error *err){
    Compiler c={.file=file,.limits=limits,.err=err,.sem=sem,.owner=0,.block=0};
    c.next_reg[0]=1;
    compile_block_items(&c,root);
    size_t exit_jump=emit_q(&c,OP_JUMP,0,root->span,0,1,0u);
    for(size_t i=0;i<sem->funcs.n&&!err->code;i++){
        Node *fn=sem->funcs.v[i];uint32_t save_owner=c.owner;int save_sim=c.sim_depth;
        c.owner=fn->fn_id;c.next_reg[c.owner]=1;c.sim_depth=0;c.block++;
        c.fn_entry[fn->fn_id]=(uint32_t)c.p.code.n;
        compile_block_items(&c,fn->as.fn.body);
        emit_q(&c,OP_RETURN,(uint8_t)TY_UNIT,fn->span,0,0);
        c.owner=save_owner;c.sim_depth=save_sim;
    }
    uint32_t end=(uint32_t)c.p.code.n;
    if(exit_jump<c.p.code.n)c.p.code.v[exit_jump].ops[0]=end;
    for(size_t i=0;i<c.calls.n&&!err->code;i++){
        CallPatch *cp=&c.calls.v[i];
        uint32_t entry=c.fn_entry[cp->fnid];
        if(!entry&&cp->fnid!=0){set_error(err,3001,file,1,1,"undefined name");break;}
        c.p.code.v[cp->ins].ops[2]=entry;
    }
    free(c.calls.v);
    if(err->code)return -1;
    c.p.entry=0;*out=c.p;return 0;
}

typedef struct { unsigned char *v; size_t n,cap; } ByteVec;
static void bv_need(ByteVec *b,size_t add){
    if(b->n+add>b->cap){size_t nc=b->cap?b->cap:256;while(nc<b->n+add)nc*=2;b->v=xrealloc(b->v,nc);b->cap=nc;}
}
static void bv_u8(ByteVec*b,uint8_t x){bv_need(b,1);b->v[b->n++]=x;}
static void bv_u16(ByteVec*b,uint16_t x){bv_need(b,2);bv_u8(b,x&255);bv_u8(b,(x>>8)&255);}
static void bv_u32(ByteVec*b,uint32_t x){bv_need(b,4);for(int i=0;i<4;i++)bv_u8(b,(uint8_t)(x>>(8*i)));}
static void bv_bytes(ByteVec*b,const void*p,size_t n){bv_need(b,n);memcpy(b->v+b->n,p,n);b->n+=n;}
static uint32_t crc32_bytes(const unsigned char *p,size_t n){
    uint32_t c=0xffffffffu;
    for(size_t i=0;i<n;i++){c^=p[i];for(int k=0;k<8;k++)c=(c>>1)^(0xedb88320u&-(int32_t)(c&1));}
    return ~c;
}
static void put_u32(unsigned char *p,uint32_t x){for(int i=0;i<4;i++)p[i]=(unsigned char)(x>>(8*i));}
static uint16_t rd_u16(const unsigned char*p){return(uint16_t)p[0]|((uint16_t)p[1]<<8);}
static uint32_t rd_u32(const unsigned char*p){return(uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}

static int qbc_encode(QbcProg *p,ByteVec *out,qnova_error *err,const char *file){
    if(p->consts.n>UINT32_MAX||p->code.n>UINT32_MAX){set_error(err,6004,file,1,1,"instruction limit exceeded");return -1;}
    ByteVec b={0};
    bv_bytes(&b,"QBC0",4);bv_u8(&b,0);bv_u8(&b,1);bv_u16(&b,0);
    bv_u32(&b,(uint32_t)p->consts.n);bv_u32(&b,(uint32_t)p->code.n);bv_u32(&b,p->entry);bv_u32(&b,0);
    for(size_t i=0;i<p->consts.n;i++){
        ConstEnt *e=&p->consts.v[i];bv_u8(&b,e->tag);bv_u8(&b,e->flags);bv_u16(&b,0);bv_u32(&b,e->len);bv_bytes(&b,e->data,e->len);
    }
    for(size_t i=0;i<p->code.n;i++){
        QIns *in=&p->code.v[i];bv_u16(&b,in->op);bv_u8(&b,in->nops);bv_u8(&b,in->flags);
        for(size_t j=0;j<in->nops;j++)bv_u32(&b,in->ops[j]);
    }
    uint32_t crc=crc32_bytes(b.v+24,b.n-24);put_u32(b.v+20,crc);*out=b;return 0;
}

static bool opcode_valid(uint16_t op){
    switch(op){
        case OP_NOP:case OP_CONST:case OP_LOAD:case OP_STORE:case OP_ADD:case OP_SUB:
        case OP_MUL:case OP_DIV:case OP_MOD:case OP_CMP_EQ:case OP_CMP_NE:case OP_CMP_LT:
        case OP_CMP_LE:case OP_CMP_GT:case OP_CMP_GE:case OP_JUMP:case OP_JUMP_IF_FALSE:
        case OP_CALL:case OP_RETURN:case OP_EMIT:case OP_GUARD_BEGIN:case OP_GUARD_END:
        case OP_OBSERVE:case OP_SIMULATE_BEGIN:case OP_SIMULATE_END:case OP_PATCH:
        case OP_QALLOC:case OP_QH:case OP_QX:case OP_QZ:case OP_QCNOT:case OP_QPROBS:
        case OP_QMEASURE:case OP_QRESET:return true;default:return false;
    }
}
static bool operand_count_valid(uint16_t op,uint8_t n,const uint32_t *ops){
    switch(op){
        case OP_NOP:case OP_GUARD_END:case OP_SIMULATE_BEGIN:case OP_SIMULATE_END:return n==0;
        case OP_JUMP:case OP_EMIT:case OP_OBSERVE:case OP_QH:case OP_QX:case OP_QZ:case OP_QRESET:return n==1;
        case OP_CONST:case OP_LOAD:case OP_STORE:case OP_JUMP_IF_FALSE:case OP_PATCH:case OP_QALLOC:case OP_QCNOT:case OP_QPROBS:return n==2;
        case OP_ADD:case OP_SUB:case OP_MUL:case OP_DIV:case OP_MOD:case OP_CMP_EQ:case OP_CMP_NE:
        case OP_CMP_LT:case OP_CMP_LE:case OP_CMP_GT:case OP_CMP_GE:case OP_GUARD_BEGIN:case OP_QMEASURE:return n==3;
        case OP_RETURN:return n<=1;
        case OP_CALL:return n>=5&&((n-5)%3)==0&&ops&&ops[4]==(uint32_t)((n-5)/3);
        default:return false;
    }
}

static int qbc_decode(const char *file,const unsigned char *buf,size_t n,
                      const qnova_limits *limits,QbcProg *out,qnova_error *err){
    if(n<24){set_error(err,5005,file,1,1,"truncated bytecode");return -1;}
    if(memcmp(buf,"QBC0",4)){set_error(err,5001,file,1,1,"invalid QBC magic");return -1;}
    if(buf[4]!=0||buf[5]!=1){set_error(err,5002,file,1,1,"unsupported QBC version");return -1;}
    uint32_t nc=rd_u32(buf+8),ni=rd_u32(buf+12),entry=rd_u32(buf+16),crc=rd_u32(buf+20);
    if(ni>limits->qbc_instructions){set_error(err,6004,file,1,1,"instruction limit exceeded");return -1;}
    if(entry>=ni&&ni){set_error(err,5005,file,1,1,"truncated bytecode");return -1;}
    if(crc32_bytes(buf+24,n-24)!=crc){set_error(err,5006,file,1,1,"checksum mismatch");return -1;}
    size_t at=24;QbcProg p={0};p.entry=entry;
    for(uint32_t i=0;i<nc;i++){
        if(at+8>n){set_error(err,5005,file,1,1,"truncated bytecode");goto bad;}
        uint8_t tag=buf[at],flags=buf[at+1];uint32_t len=rd_u32(buf+at+4);at+=8;
        if(at+len>n){set_error(err,5005,file,1,1,"truncated bytecode");goto bad;}
        unsigned char *d=xmalloc(len?len:1);if(len)memcpy(d,buf+at,len);at+=len;
        const_push(&p.consts,(ConstEnt){tag,flags,len,d});
    }
    for(uint32_t i=0;i<ni;i++){
        if(at+4>n){set_error(err,5005,file,1,1,"truncated bytecode");goto bad;}
        uint16_t op=rd_u16(buf+at);uint8_t nop=buf[at+2],flags=buf[at+3];at+=4;
        if(!opcode_valid(op)){set_error(err,5004,file,1,1,"unknown opcode");goto bad;}
        if(at+(size_t)nop*4>n){set_error(err,5005,file,1,1,"truncated bytecode");goto bad;}
        uint32_t *ops=xcalloc(nop?nop:1,sizeof(uint32_t));
        for(uint8_t j=0;j<nop;j++){ops[j]=rd_u32(buf+at);at+=4;}
        if(!operand_count_valid(op,nop,ops)){free(ops);set_error(err,5005,file,1,1,"truncated bytecode");goto bad;}
        if(op==OP_CONST&&ops[1]>=p.consts.n){free(ops);set_error(err,5003,file,1,1,"invalid constant-pool index");goto bad;}
        if((op==OP_JUMP&&ops[0]>ni)||(op==OP_JUMP_IF_FALSE&&ops[1]>ni)||
           (op==OP_GUARD_BEGIN&&ops[1]>ni)||(op==OP_CALL&&ops[2]>=ni)){
            free(ops);set_error(err,5005,file,1,1,"truncated bytecode");goto bad;
        }
        qir_push(&p.code,(QIns){op,flags,nop,ops,{1,1,1,1},0,0});
    }
    if(at!=n){set_error(err,5005,file,1,1,"truncated bytecode");goto bad;}
    *out=p;return 0;
bad:
    for(size_t i=0;i<p.consts.n;i++) free(p.consts.v[i].data);
    free(p.consts.v);
    for(size_t i=0;i<p.code.n;i++) free(p.code.v[i].ops);
    free(p.code.v);
    return -1;
}

static void qbc_free(QbcProg *p){
    for(size_t i=0;i<p->consts.n;i++) free(p->consts.v[i].data);
    free(p->consts.v);
    for(size_t i=0;i<p->code.n;i++) free(p->code.v[i].ops);
    free(p->code.v);
    memset(p,0,sizeof(*p));
}


typedef struct { const unsigned char *p; uint32_t n; } StrVal;
typedef struct { double p0,p1; } ProbVal;
typedef struct {
    Type t;
    union { int64_t i; double r; bool b; StrVal s; uint32_t q; ProbVal probs; } v;
} Value;

typedef struct { uint32_t id; Value val; } Slot;
typedef struct Frame Frame;
struct Frame {
    uint32_t fnid;
    Slot *bind; size_t nbind,cbind;
    Slot *reg; size_t nreg,creg;
    size_t return_ip;
    uint32_t return_reg;
    Type return_type;
    Frame *prev;
};

typedef struct {
    size_t n,dim;
    double complex *amp;
} QState;

typedef struct FrameSnap FrameSnap;
struct FrameSnap {
    Frame *frame;
    Slot *bind; size_t nbind;
    Slot *reg; size_t nreg;
    FrameSnap *next;
};
typedef struct SimSnap SimSnap;
struct SimSnap {
    Frame *top;
    FrameSnap *frames;
    QState qs;
    SimSnap *prev;
};

typedef struct {
    const char *file;
    QbcProg *p;
    const qnova_limits *limits;
    qnova_error *err;
    FILE *out;
    Frame *frame;
    QState qs;
    SimSnap *sim;
    int sim_depth;
    uint64_t rng;
    size_t steps;
    size_t calls;
} VM;

static Value v_unit(void){Value v={.t=TY_UNIT};return v;}
static Value v_int(int64_t x){Value v={.t=TY_INT};v.v.i=x;return v;}
static Value v_real(double x){Value v={.t=TY_REAL};v.v.r=x;return v;}
static Value v_bool(bool x){Value v={.t=TY_BOOL};v.v.b=x;return v;}
static Value v_bit(int x){Value v={.t=TY_BIT};v.v.i=x?1:0;return v;}
static Value v_q(uint32_t q){Value v={.t=TY_QBIT};v.v.q=q;return v;}
static Value v_probs(double p0,double p1){Value v={.t=TY_PROBS};v.v.probs=(ProbVal){p0,p1};return v;}

static Slot *slot_find(Slot *v,size_t n,uint32_t id){
    for(size_t i=0;i<n;i++) if(v[i].id==id) return &v[i];
    return NULL;
}
static void slot_set(Slot **v,size_t *n,size_t *cap,uint32_t id,Value val){
    Slot *s=slot_find(*v,*n,id);if(s){s->val=val;return;}
    if(*n==*cap){*cap=*cap?*cap*2:16;*v=xrealloc(*v,*cap*sizeof(**v));}
    (*v)[(*n)++]=(Slot){id,val};
}
static Frame *frame_for_reg(VM *vm,uint32_t reg){
    uint32_t owner=reg>>24;
    for(Frame *f=vm->frame;f;f=f->prev)if(f->fnid==owner)return f;
    return NULL;
}
static bool reg_get(VM *vm,uint32_t reg,Value *out){
    Frame *f=frame_for_reg(vm,reg);if(!f)return false;
    Slot *s=slot_find(f->reg,f->nreg,reg);if(!s)return false;*out=s->val;return true;
}
static void reg_set(VM *vm,uint32_t reg,Value val){
    Frame *f=frame_for_reg(vm,reg);
    if(!f)f=vm->frame;
    slot_set(&f->reg,&f->nreg,&f->creg,reg,val);
}
static Slot *binding_find(VM *vm,uint32_t sym,Frame **owner){
    for(Frame *f=vm->frame;f;f=f->prev){
        Slot *s=slot_find(f->bind,f->nbind,sym);
        if(s){if(owner)*owner=f;return s;}
    }
    return NULL;
}
static void binding_declare(VM *vm,uint32_t sym,Value val){
    slot_set(&vm->frame->bind,&vm->frame->nbind,&vm->frame->cbind,sym,val);
}
static void free_frame(Frame *f){if(!f)return;free(f->bind);free(f->reg);free(f);}

static bool convert_value(Value in,Type dst,Value *out){
    if(in.t==dst){*out=in;return true;}
    if(dst==TY_REAL&&in.t==TY_INT){*out=v_real((double)in.v.i);return true;}
    return false;
}

static double as_double(Value v){return v.t==TY_REAL?v.v.r:(double)v.v.i;}

static int qstate_alloc(VM *vm,int init,uint32_t *idx){
    if(vm->qs.n>=vm->limits->qbits){set_error(vm->err,4006,vm->file,1,1,"qbit resource limit exceeded");return -1;}
    if(vm->qs.n>=8*sizeof(size_t)-1){set_error(vm->err,4006,vm->file,1,1,"qbit resource limit exceeded");return -1;}
    size_t olddim=vm->qs.dim?vm->qs.dim:1,newdim=olddim*2;
    double complex *na=xcalloc(newdim,sizeof(*na));
    if(!vm->qs.amp){vm->qs.amp=xcalloc(1,sizeof(*vm->qs.amp));vm->qs.amp[0]=1.0+0.0*I;}
    for(size_t i=0;i<olddim;i++)na[i+(init?olddim:0)]=vm->qs.amp[i];
    free(vm->qs.amp);vm->qs.amp=na;*idx=(uint32_t)vm->qs.n;vm->qs.n++;vm->qs.dim=newdim;return 0;
}
static double qnorm(QState *q){double s=0;for(size_t i=0;i<q->dim;i++){double a=cabs(q->amp[i]);s+=a*a;}return s;}
static int qcheck(VM *vm){
    double n=qnorm(&vm->qs);
    if(!isfinite(n)||fabs(n-1.0)>1e-8){set_error(vm->err,4004,vm->file,1,1,"invalid or non-normalized state");return -1;}
    return 0;
}
static int qgate1(VM *vm,uint32_t q,int gate){
    if(q>=vm->qs.n||qcheck(vm)) return -1;
    size_t m=(size_t)1<<q;
    if(gate==OP_QH){
        const double s=1.0/sqrt(2.0);
        for(size_t i=0;i<vm->qs.dim;i++)if(!(i&m)){
            size_t j=i|m;double complex a=vm->qs.amp[i],b=vm->qs.amp[j];
            vm->qs.amp[i]=(a+b)*s;vm->qs.amp[j]=(a-b)*s;
        }
    }else if(gate==OP_QX){
        for(size_t i=0;i<vm->qs.dim;i++)if(!(i&m)){size_t j=i|m;double complex t=vm->qs.amp[i];vm->qs.amp[i]=vm->qs.amp[j];vm->qs.amp[j]=t;}
    }else if(gate==OP_QZ){
        for(size_t i=0;i<vm->qs.dim;i++)if(i&m)vm->qs.amp[i]=-vm->qs.amp[i];
    }
    return qcheck(vm);
}
static int qcnot(VM *vm,uint32_t c,uint32_t t){
    if(c>=vm->qs.n||t>=vm->qs.n||c==t||qcheck(vm))return -1;
    size_t cm=(size_t)1<<c,tm=(size_t)1<<t;
    for(size_t i=0;i<vm->qs.dim;i++)if((i&cm)&&!(i&tm)){
        size_t j=i|tm;double complex z=vm->qs.amp[i];vm->qs.amp[i]=vm->qs.amp[j];vm->qs.amp[j]=z;
    }
    return qcheck(vm);
}
static int qprobs(VM *vm,uint32_t q,double *p0,double *p1){
    if(q>=vm->qs.n||qcheck(vm)) return -1;
    size_t m=(size_t)1<<q;
    double a=0,b=0;
    for(size_t i=0;i<vm->qs.dim;i++){double x=cabs(vm->qs.amp[i]);x*=x;if(i&m)b+=x;else a+=x;}
    *p0=a;*p1=b;return 0;
}
static uint64_t rng_next(VM *vm){
    uint64_t x=vm->rng;if(!x)x=0x9e3779b97f4a7c15ULL;
    x^=x>>12;x^=x<<25;x^=x>>27;vm->rng=x;return x*2685821657736338717ULL;
}
static double rng_unit(VM *vm){return (rng_next(vm)>>11)*(1.0/9007199254740992.0);}
static int qmeasure(VM *vm,uint32_t q,int *bit){
    double p0,p1;if(qprobs(vm,q,&p0,&p1))return -1;
    double r=rng_unit(vm);int b=(r>=p0)?1:0;double keep=b?p1:p0;
    if(keep<=0){b=p0>0?0:1;keep=b?p1:p0;}
    size_t m=(size_t)1<<q;double scale=1.0/sqrt(keep);
    for(size_t i=0;i<vm->qs.dim;i++){
        if(((i&m)?1:0)!=b)vm->qs.amp[i]=0;
        else vm->qs.amp[i]*=scale;
    }
    *bit=b;return qcheck(vm);
}
static int qreset(VM *vm,uint32_t q){
    int b=0;if(qmeasure(vm,q,&b))return -1;if(b)return qgate1(vm,q,OP_QX);return 0;
}

static Value const_value(VM *vm,uint32_t idx,Type want,bool *ok){
    *ok=false;if(idx>=vm->p->consts.n)return v_unit();ConstEnt *e=&vm->p->consts.v[idx];
    if(want==TY_STRING){
        if(e->tag!=0x03)return v_unit();
        Value v={.t=TY_STRING};v.v.s=(StrVal){e->data,e->len};*ok=true;return v;
    }
    if(want==TY_REAL){
        if(e->tag!=0x02||e->len!=8) return v_unit();
        uint64_t u=0;
        for(int i=0;i<8;i++) u|=(uint64_t)e->data[i]<<(8*i);
        double d;memcpy(&d,&u,8);*ok=true;return v_real(d);
    }
    if((want==TY_INT||want==TY_BOOL||want==TY_BIT)&&e->tag==0x01&&e->len==8){
        uint64_t u=0;for(int i=0;i<8;i++)u|=(uint64_t)e->data[i]<<(8*i);int64_t x=(int64_t)u;*ok=true;
        if(want==TY_BOOL) return v_bool(x!=0);
        if(want==TY_BIT) return v_bit(x!=0);
        return v_int(x);
    }
    return v_unit();
}

static void print_value(FILE *out,Value v){
    switch(v.t){
        case TY_INT:fprintf(out,"%" PRId64,v.v.i);break;
        case TY_REAL:fprintf(out,"%.15g",v.v.r);break;
        case TY_BOOL:fputs(v.v.b?"true":"false",out);break;
        case TY_BIT:fprintf(out,"%" PRId64,(int64_t)(v.v.i?1:0));break;
        case TY_STRING:fwrite(v.v.s.p,1,v.v.s.n,out);break;
        case TY_PROBS:fprintf(out,"p0=%.3f,p1=%.3f",v.v.probs.p0,v.v.probs.p1);break;
        case TY_UNIT:fputs("unit",out);break;
        default:fputs("<value>",out);break;
    }
}
static bool value_equal(Value a,Value b){
    if(numeric(a.t)&&numeric(b.t))return as_double(a)==as_double(b);
    if(a.t!=b.t)return false;
    if(a.t==TY_BOOL)return a.v.b==b.v.b;
    if(a.t==TY_BIT||a.t==TY_INT)return a.v.i==b.v.i;
    if(a.t==TY_REAL)return a.v.r==b.v.r;
    if(a.t==TY_STRING)return a.v.s.n==b.v.s.n&&!memcmp(a.v.s.p,b.v.s.p,a.v.s.n);
    return false;
}

static FrameSnap *snapshot_frames(Frame *top){
    FrameSnap *head=NULL,*tail=NULL;
    for(Frame *f=top;f;f=f->prev){
        FrameSnap *s=xcalloc(1,sizeof(*s));s->frame=f;s->nbind=f->nbind;s->nreg=f->nreg;
        if(s->nbind){s->bind=xmalloc(s->nbind*sizeof(*s->bind));memcpy(s->bind,f->bind,s->nbind*sizeof(*s->bind));}
        if(s->nreg){s->reg=xmalloc(s->nreg*sizeof(*s->reg));memcpy(s->reg,f->reg,s->nreg*sizeof(*s->reg));}
        if(!head)head=s;else tail->next=s;tail=s;
    }
    return head;
}
static QState snapshot_qs(QState *q){
    QState r={q->n,q->dim,NULL};if(q->dim){r.amp=xmalloc(q->dim*sizeof(*r.amp));memcpy(r.amp,q->amp,q->dim*sizeof(*r.amp));}return r;
}
static void freesnap_frames(FrameSnap *s){
    while(s){FrameSnap*n=s->next;free(s->bind);free(s->reg);free(s);s=n;}
}
static void sim_begin(VM *vm){
    SimSnap *s=xcalloc(1,sizeof(*s));s->top=vm->frame;s->frames=snapshot_frames(vm->frame);s->qs=snapshot_qs(&vm->qs);s->prev=vm->sim;vm->sim=s;vm->sim_depth++;
}
static void sim_restore(VM *vm){
    SimSnap *s=vm->sim;if(!s)return;
    while(vm->frame&&vm->frame!=s->top){Frame *x=vm->frame;vm->frame=x->prev;free_frame(x);}
    for(FrameSnap *fs=s->frames;fs;fs=fs->next){
        Frame *f=fs->frame;free(f->bind);free(f->reg);f->bind=fs->bind;f->nbind=fs->nbind;f->cbind=fs->nbind;
        f->reg=fs->reg;f->nreg=fs->nreg;f->creg=fs->nreg;fs->bind=NULL;fs->reg=NULL;
    }
    free(vm->qs.amp);vm->qs=s->qs;s->qs.amp=NULL;
    vm->sim=s->prev;if(vm->sim_depth>0)vm->sim_depth--;
    freesnap_frames(s->frames);free(s->qs.amp);free(s);
}
static void vm_runtime_fail(VM *vm,int code,const char *msg){
    while(vm->sim)sim_restore(vm);
    set_error(vm->err,code,vm->file,1,1,msg);
}

static int vm_qref(VM *vm,uint32_t sym,uint32_t *q){
    Slot *s=binding_find(vm,sym,NULL);
    if(!s||s->val.t!=TY_QBIT){vm_runtime_fail(vm,4001,"unknown qbit");return -1;}
    *q=s->val.v.q;return 0;
}

static int vm_exec(const char *file,QbcProg *p,const qnova_limits *limits,
                   uint64_t seed,int deterministic,FILE *out,qnova_error *err){
    Frame *g=xcalloc(1,sizeof(*g));g->fnid=0;
    struct timespec ts;clock_gettime(CLOCK_REALTIME,&ts);
    uint64_t init=deterministic?seed:((uint64_t)ts.tv_sec<<32)^(uint64_t)ts.tv_nsec^0xa0761d6478bd642fULL;
    VM vm={.file=file,.p=p,.limits=limits,.err=err,.out=out,.frame=g,.rng=init};
    vm.qs.dim=1;vm.qs.amp=xcalloc(1,sizeof(*vm.qs.amp));vm.qs.amp[0]=1.0+0.0*I;
    size_t ip=p->entry;
    while(ip<p->code.n&&!err->code){
        if(++vm.steps>limits->execution_steps){vm_runtime_fail(&vm,6004,"instruction limit exceeded");break;}
        QIns *in=&p->code.v[ip];uint32_t *o=in->ops;Value a,b,d;bool ok=true;
        switch(in->op){
            case OP_NOP:ip++;break;
            case OP_CONST:
                d=const_value(&vm,o[1],(Type)in->flags,&ok);
                if(!ok){vm_runtime_fail(&vm,5003,"invalid constant-pool index");break;}
                reg_set(&vm,o[0],d);ip++;break;
            case OP_LOAD:{
                Slot *s=binding_find(&vm,o[1],NULL);if(!s){vm_runtime_fail(&vm,3001,"undefined name");break;}
                reg_set(&vm,o[0],s->val);ip++;break;
            }
            case OP_STORE:
                if(!reg_get(&vm,o[1],&a)||!convert_value(a,(Type)in->flags,&d)){vm_runtime_fail(&vm,3003,"type mismatch");break;}
                binding_declare(&vm,o[0],d);ip++;break;
            case OP_PATCH:{
                Slot *s=binding_find(&vm,o[0],NULL);if(!s){vm_runtime_fail(&vm,7004,"patch transaction failed");break;}
                if(!reg_get(&vm,o[1],&a)||!convert_value(a,(Type)in->flags,&d)){vm_runtime_fail(&vm,7004,"patch transaction failed");break;}
                s->val=d;ip++;break;
            }
            case OP_ADD:case OP_SUB:case OP_MUL:case OP_DIV:case OP_MOD:
                if(!reg_get(&vm,o[1],&a)||!reg_get(&vm,o[2],&b)||!numeric(a.t)||!numeric(b.t)){vm_runtime_fail(&vm,3003,"type mismatch");break;}
                if(in->op==OP_DIV&&((b.t==TY_REAL&&b.v.r==0.0)||(b.t==TY_INT&&b.v.i==0))){vm_runtime_fail(&vm,6001,"division by zero");break;}
                if(in->op==OP_MOD&&(b.t!=TY_INT||a.t!=TY_INT||b.v.i==0)){if(b.t==TY_INT&&a.t==TY_INT)vm_runtime_fail(&vm,6001,"division by zero");else vm_runtime_fail(&vm,3003,"type mismatch");break;}
                if((Type)in->flags==TY_REAL){
                    double x=as_double(a),y=as_double(b),z=0;
                    if(in->op==OP_ADD)z=x+y;else if(in->op==OP_SUB)z=x-y;else if(in->op==OP_MUL)z=x*y;else z=x/y;
                    d=v_real(z);
                }else{
                    int64_t x=a.v.i,y=b.v.i,z=0;
                    if(in->op==OP_ADD)z=x+y;else if(in->op==OP_SUB)z=x-y;else if(in->op==OP_MUL)z=x*y;else if(in->op==OP_DIV)z=x/y;else z=x%y;
                    d=v_int(z);
                }
                reg_set(&vm,o[0],d);ip++;break;
            case OP_CMP_EQ:case OP_CMP_NE:case OP_CMP_LT:case OP_CMP_LE:case OP_CMP_GT:case OP_CMP_GE:{
                if(!reg_get(&vm,o[1],&a)||!reg_get(&vm,o[2],&b)){vm_runtime_fail(&vm,3003,"type mismatch");break;}
                bool r=false;
                if(in->op==OP_CMP_EQ||in->op==OP_CMP_NE){r=value_equal(a,b);if(in->op==OP_CMP_NE)r=!r;}
                else{
                    if(!numeric(a.t)||!numeric(b.t)){vm_runtime_fail(&vm,3003,"type mismatch");break;}
                    double x=as_double(a),y=as_double(b);
                    if(in->op==OP_CMP_LT)r=x<y;else if(in->op==OP_CMP_LE)r=x<=y;else if(in->op==OP_CMP_GT)r=x>y;else r=x>=y;
                }
                reg_set(&vm,o[0],v_bool(r));ip++;break;
            }
            case OP_JUMP:ip=o[0];break;
            case OP_JUMP_IF_FALSE:
                if(!reg_get(&vm,o[0],&a)){vm_runtime_fail(&vm,3003,"type mismatch");break;}
                if(in->flags&0x80){
                    if(a.t!=TY_INT||a.v.i<0||(uint64_t)a.v.i>limits->repeat_iterations){vm_runtime_fail(&vm,6005,"repeat limit exceeded");break;}
                    if(a.v.i==0)ip=o[1];else ip++;
                }else{
                    if(a.t!=TY_BOOL){vm_runtime_fail(&vm,3004,"condition is not Boolean");break;}
                    ip=a.v.b?ip+1:o[1];
                }
                break;
            case OP_GUARD_BEGIN:
                if(!reg_get(&vm,o[0],&a)||a.t!=TY_BOOL){vm_runtime_fail(&vm,3004,"condition is not Boolean");break;}
                if(a.v.b)ip++;else if(o[2])ip=o[1];else vm_runtime_fail(&vm,7001,"guard denied");
                break;
            case OP_GUARD_END:ip++;break;
            case OP_EMIT:
            case OP_OBSERVE:
                if(!reg_get(&vm,o[0],&a)){vm_runtime_fail(&vm,3001,"undefined name");break;}
                if(vm.sim_depth)fputs("[sim] ",out);
                if(in->op==OP_OBSERVE&&a.t!=TY_PROBS)fputs("observe:",out);
                print_value(out,a);fputc('\n',out);ip++;break;
            case OP_SIMULATE_BEGIN:sim_begin(&vm);ip++;break;
            case OP_SIMULATE_END:sim_restore(&vm);ip++;break;
            case OP_CALL:{
                if(vm.calls>=limits->call_depth){vm_runtime_fail(&vm,6002,"call-depth limit exceeded");break;}
                uint32_t argc=o[4];Frame *nf=xcalloc(1,sizeof(*nf));nf->fnid=o[1];nf->return_ip=ip+1;nf->return_reg=o[0];nf->return_type=(Type)o[3];nf->prev=vm.frame;
                for(uint32_t i=0;i<argc;i++){
                    uint32_t ps=o[5+3*i];Type pt=(Type)o[5+3*i+1];uint32_t ar=o[5+3*i+2];
                    if(!reg_get(&vm,ar,&a)||!convert_value(a,pt,&d)){free_frame(nf);vm_runtime_fail(&vm,3003,"type mismatch");goto call_done;}
                    slot_set(&nf->bind,&nf->nbind,&nf->cbind,ps,d);
                }
                vm.frame=nf;vm.calls++;ip=o[2];
call_done:       break;
            }
            case OP_RETURN:{
                if(!vm.frame->prev){vm_runtime_fail(&vm,6003,"return outside function");break;}
                Value rv=v_unit();if(in->nops&& !reg_get(&vm,o[0],&rv)){vm_runtime_fail(&vm,3007,"return type mismatch");break;}
                Frame *old=vm.frame;Frame *caller=old->prev;size_t rip=old->return_ip;uint32_t rr=old->return_reg;Type rt=old->return_type;
                if(!convert_value(rv,rt,&d)){vm_runtime_fail(&vm,3007,"return type mismatch");break;}
                vm.frame=caller;free_frame(old);if(vm.calls)vm.calls--;reg_set(&vm,rr,d);ip=rip;break;
            }
            case OP_QALLOC:{
                uint32_t qi;if(qstate_alloc(&vm,(int)o[1],&qi))break;binding_declare(&vm,o[0],v_q(qi));ip++;break;
            }
            case OP_QH:case OP_QX:case OP_QZ:{
                uint32_t q;if(vm_qref(&vm,o[0],&q)||qgate1(&vm,q,in->op))break;ip++;break;
            }
            case OP_QCNOT:{
                uint32_t q1,q2;if(vm_qref(&vm,o[0],&q1)||vm_qref(&vm,o[1],&q2))break;
                if(q1==q2){vm_runtime_fail(&vm,4003,"CNOT control equals target");break;}
                if(qcnot(&vm,q1,q2)) break;
                ip++;
                break;
            }
            case OP_QPROBS:{
                uint32_t q;double p0,p1;if(vm_qref(&vm,o[1],&q)||qprobs(&vm,q,&p0,&p1))break;
                reg_set(&vm,o[0],v_probs(p0,p1));ip++;break;
            }
            case OP_QMEASURE:{
                uint32_t q;int bit;if(vm_qref(&vm,o[1],&q)||qmeasure(&vm,q,&bit))break;
                d=v_bit(bit);reg_set(&vm,o[0],d);
                if(o[2]!=U32_NONE){Slot *s=binding_find(&vm,o[2],NULL);if(s){if(s->val.t!=TY_BIT){vm_runtime_fail(&vm,4005,"invalid measurement target");break;}s->val=d;}else binding_declare(&vm,o[2],d);}
                ip++;break;
            }
            case OP_QRESET:{
                uint32_t q;if(vm_qref(&vm,o[0],&q)||qreset(&vm,q))break;ip++;break;
            }
            default:vm_runtime_fail(&vm,5004,"unknown opcode");break;
        }
    }
    while(vm.sim)sim_restore(&vm);
    while(vm.frame){Frame *x=vm.frame;vm.frame=x->prev;free_frame(x);}
    free(vm.qs.amp);
    return err->code?-1:0;
}


typedef struct {
    char *src;
    size_t src_n;
    TokenVec tv;
    Node *root;
    Sem sem;
    QbcProg prog;
    bool has_tokens,has_ast,has_sem,has_prog;
} Pipeline;

static void free_node(Node *n){
    if(!n)return;
    switch(n->kind){
        case N_PROGRAM:case N_BLOCK:
            for(size_t i=0;i<n->as.list.items.n;i++)free_node(n->as.list.items.v[i]);
            free(n->as.list.items.v);break;
        case N_LET:free(n->as.let.name);free_node(n->as.let.init);break;
        case N_FUNCTION:
            free(n->as.fn.name);
            for(size_t i=0;i<n->as.fn.params.n;i++)free(n->as.fn.params.v[i].name);
            free(n->as.fn.params.v);free_node(n->as.fn.body);break;
        case N_QBIT_DECL:free(n->as.qbit.name);break;
        case N_RETURN:case N_EMIT:case N_OBSERVE:case N_EXPR_STMT:free_node(n->as.one.expr);break;
        case N_WHEN:case N_GUARD:
            free_node(n->as.branch.cond);free_node(n->as.branch.yes);free_node(n->as.branch.no);break;
        case N_REPEAT:free_node(n->as.repeat.count);free_node(n->as.repeat.body);break;
        case N_SIMULATE:free_node(n->as.simulate.body);break;
        case N_PATCH:free(n->as.patch.name);free_node(n->as.patch.expr);break;
        case N_GATE:free(n->as.gate.a);free(n->as.gate.b);break;
        case N_MEASURE:free(n->as.measure.q);free(n->as.measure.target);break;
        case N_RESET:free(n->as.reset.q);break;
        case N_BINARY:free_node(n->as.binary.a);free_node(n->as.binary.b);break;
        case N_UNARY:free_node(n->as.unary.expr);break;
        case N_CALL:
            free(n->as.call.name);for(size_t i=0;i<n->as.call.args.n;i++)free_node(n->as.call.args.v[i]);
            free(n->as.call.args.v);break;
        case N_PROBS:free(n->as.probs.q);break;
        case N_LITERAL:if(n->as.lit.tag==TY_STRING)free(n->as.lit.s);break;
        case N_NAME:free(n->as.name.name);break;
    }
    free(n);
}
static void pipeline_free(Pipeline *p){
    if(p->has_prog)qbc_free(&p->prog);
    if(p->has_sem)free(p->sem.funcs.v);
    if(p->has_ast)free_node(p->root);
    if(p->has_tokens)free_tokens(&p->tv);
    free(p->src);memset(p,0,sizeof(*p));
}
static int pipeline_build(const char *path,const qnova_limits *limits,
                          bool need_compile,Pipeline *p,qnova_error *err){
    if(read_entire_file(path,limits->source_bytes,&p->src,&p->src_n,err))return -1;
    if(lex_source(path,p->src,p->src_n,limits,&p->tv,err)) return -1;
    p->has_tokens=true;
    p->root=parse_program(path,&p->tv,limits,err);if(err->code)return -1;p->has_ast=true;
    if(semantic_analyze(path,p->root,limits,&p->sem,err)) return -1;
    p->has_sem=true;
    if(need_compile){if(compile_program(path,p->root,&p->sem,limits,&p->prog,err))return -1;p->has_prog=true;}
    return 0;
}

static const char *node_name(NodeKind k){
    switch(k){
        case N_PROGRAM:return"Program";case N_BLOCK:return"Block";case N_LET:return"LetDecl";
        case N_FUNCTION:return"FunctionDecl";case N_QBIT_DECL:return"QbitDecl";case N_RETURN:return"ReturnStmt";
        case N_EMIT:return"EmitStmt";case N_WHEN:return"WhenStmt";case N_REPEAT:return"RepeatStmt";
        case N_GUARD:return"GuardStmt";case N_OBSERVE:return"ObserveStmt";case N_SIMULATE:return"SimulateStmt";
        case N_PATCH:return"PatchStmt";case N_GATE:return"GateStmt";case N_MEASURE:return"MeasureStmt";
        case N_RESET:return"ResetStmt";case N_EXPR_STMT:return"ExpressionStmt";case N_BINARY:return"BinaryExpr";
        case N_UNARY:return"UnaryExpr";case N_CALL:return"CallExpr";case N_PROBS:return"ProbabilityExpr";
        case N_LITERAL:return"LiteralExpr";case N_NAME:return"NameExpr";default:return"?";
    }
}
static void ast_indent(FILE*out,int d){for(int i=0;i<d;i++)fputs("  ",out);}
static void dump_ast_node(FILE*out,Node*n,int d){
    if(!n) return;
    ast_indent(out,d);
    fprintf(out,"%s type=%s @%u:%u",node_name(n->kind),type_name(n->type),n->span.line,n->span.col);
    switch(n->kind){
        case N_LET:fprintf(out," name=%s sym=%u",n->as.let.name,n->sym_id);break;
        case N_FUNCTION:fprintf(out," name=%s fn=%u ret=%s",n->as.fn.name,n->fn_id,type_name(n->as.fn.ret));break;
        case N_QBIT_DECL:fprintf(out," name=%s sym=%u init=|%d>",n->as.qbit.name,n->sym_id,n->as.qbit.init);break;
        case N_PATCH:fprintf(out," name=%s sym=%u",n->as.patch.name,n->sym_id);break;
        case N_GATE:fprintf(out," op=%s a=%s",token_name(n->as.gate.op),n->as.gate.a);if(n->as.gate.b)fprintf(out," b=%s",n->as.gate.b);break;
        case N_MEASURE:fprintf(out," q=%s",n->as.measure.q);if(n->as.measure.target)fprintf(out," target=%s",n->as.measure.target);break;
        case N_RESET:fprintf(out," q=%s",n->as.reset.q);break;
        case N_BINARY:fprintf(out," op=%s",token_name(n->as.binary.op));break;
        case N_UNARY:fprintf(out," op=%s",token_name(n->as.unary.op));break;
        case N_CALL:fprintf(out," name=%s fn=%u",n->as.call.name,n->as.call.target_fn);break;
        case N_PROBS:fprintf(out," q=%s",n->as.probs.q);break;
        case N_NAME:fprintf(out," name=%s sym=%u",n->as.name.name,n->sym_id);break;
        case N_LITERAL:
            if(n->as.lit.tag==TY_INT)fprintf(out," value=%" PRId64,n->as.lit.i);
            else if(n->as.lit.tag==TY_REAL)fprintf(out," value=%.15g",n->as.lit.r);
            else if(n->as.lit.tag==TY_BOOL)fprintf(out," value=%s",n->as.lit.b?"true":"false");
            else if(n->as.lit.tag==TY_STRING)fprintf(out," value=\"%s\"",n->as.lit.s);
            break;
        default:break;
    }
    fputc('\n',out);
    switch(n->kind){
        case N_PROGRAM:case N_BLOCK:for(size_t i=0;i<n->as.list.items.n;i++)dump_ast_node(out,n->as.list.items.v[i],d+1);break;
        case N_LET:dump_ast_node(out,n->as.let.init,d+1);break;
        case N_FUNCTION:
            for(size_t i=0;i<n->as.fn.params.n;i++){ast_indent(out,d+1);fprintf(out,"Parameter %s:%s sym=%u\n",n->as.fn.params.v[i].name,type_name(n->as.fn.params.v[i].type),n->as.fn.params.v[i].sym_id);}
            dump_ast_node(out,n->as.fn.body,d+1);break;
        case N_RETURN:case N_EMIT:case N_OBSERVE:case N_EXPR_STMT:dump_ast_node(out,n->as.one.expr,d+1);break;
        case N_WHEN:case N_GUARD:dump_ast_node(out,n->as.branch.cond,d+1);dump_ast_node(out,n->as.branch.yes,d+1);dump_ast_node(out,n->as.branch.no,d+1);break;
        case N_REPEAT:dump_ast_node(out,n->as.repeat.count,d+1);dump_ast_node(out,n->as.repeat.body,d+1);break;
        case N_SIMULATE:dump_ast_node(out,n->as.simulate.body,d+1);break;
        case N_PATCH:dump_ast_node(out,n->as.patch.expr,d+1);break;
        case N_BINARY:dump_ast_node(out,n->as.binary.a,d+1);dump_ast_node(out,n->as.binary.b,d+1);break;
        case N_UNARY:dump_ast_node(out,n->as.unary.expr,d+1);break;
        case N_CALL:for(size_t i=0;i<n->as.call.args.n;i++)dump_ast_node(out,n->as.call.args.v[i],d+1);break;
        default:break;
    }
}
static const char *effect_name(uint32_t e){
    if(!e) return "pure";
    if(e==EFF_MUTATE) return "mutate";
    if(e==EFF_OUTPUT) return "output";
    if(e==EFF_QUANTUM) return "quantum";
    if(e==EFF_OBSERVE) return "observe";
    return "mixed";
}

int qnova_check_file(const char *path,const qnova_limits *limits,qnova_error *err){
    Pipeline p={0};int rc=pipeline_build(path,limits,false,&p,err);pipeline_free(&p);return rc;
}
int qnova_dump_tokens(const char *path,const qnova_limits *limits,FILE*out,qnova_error *err){
    char *src=NULL;size_t n=0;TokenVec tv={0};
    if(read_entire_file(path,limits->source_bytes,&src,&n,err))return -1;
    int rc=lex_source(path,src,n,limits,&tv,err);
    if(!rc)for(size_t i=0;i<tv.n;i++)fprintf(out,"%u:%u %-16s %s\n",tv.v[i].span.line,tv.v[i].span.col,token_name(tv.v[i].kind),tv.v[i].text);
    free_tokens(&tv);free(src);return rc;
}
int qnova_dump_ast(const char *path,const qnova_limits *limits,FILE*out,qnova_error *err){
    Pipeline p={0};if(pipeline_build(path,limits,false,&p,err)){pipeline_free(&p);return -1;}
    dump_ast_node(out,p.root,0);pipeline_free(&p);return 0;
}
int qnova_dump_qir(const char *path,const qnova_limits *limits,FILE*out,qnova_error *err){
    Pipeline p={0};if(pipeline_build(path,limits,true,&p,err)){pipeline_free(&p);return -1;}
    for(size_t i=0;i<p.prog.code.n;i++){
        QIns *in=&p.prog.code.v[i];
        fprintf(out,"%04zu bb=%u %-16s flags=%u effect=%s @%u:%u",i,in->block,opcode_name(in->op),in->flags,effect_name(in->effect),in->span.line,in->span.col);
        for(size_t j=0;j<in->nops;j++)fprintf(out," %u",in->ops[j]);
        fputc('\n',out);
    }
    pipeline_free(&p);return 0;
}
int qnova_compile_file(const char *src_path,const char *out_path,const qnova_limits *limits,qnova_error *err){
    Pipeline p={0};ByteVec b={0};
    if(pipeline_build(src_path,limits,true,&p,err)){pipeline_free(&p);return -1;}
    if(qbc_encode(&p.prog,&b,err,src_path)){pipeline_free(&p);free(b.v);return -1;}
    FILE *f=fopen(out_path,"wb");if(!f){set_error(err,5005,out_path,1,1,"truncated bytecode");pipeline_free(&p);free(b.v);return -1;}
    size_t w=fwrite(b.v,1,b.n,f);int bad=(w!=b.n||fclose(f)!=0);free(b.v);pipeline_free(&p);
    if(bad){set_error(err,5005,out_path,1,1,"truncated bytecode");return -1;}return 0;
}
int qnova_run_file(const char *path,const qnova_limits *limits,uint64_t seed,int det,FILE*out,qnova_error *err){
    Pipeline p={0};if(pipeline_build(path,limits,true,&p,err)){pipeline_free(&p);return -1;}
    int rc=vm_exec(path,&p.prog,limits,seed,det,out,err);pipeline_free(&p);return rc;
}
static int load_qbc_file(const char *path,const qnova_limits *limits,QbcProg *p,qnova_error *err){
    char *buf=NULL;size_t n=0;
    if(read_entire_file(path,64*1024*1024,&buf,&n,err))return -1;
    int rc=qbc_decode(path,(unsigned char*)buf,n,limits,p,err);free(buf);return rc;
}
int qnova_run_qbc(const char *path,const qnova_limits *limits,uint64_t seed,int det,FILE*out,qnova_error *err){
    QbcProg p={0};if(load_qbc_file(path,limits,&p,err))return -1;
    int rc=vm_exec(path,&p,limits,seed,det,out,err);qbc_free(&p);return rc;
}
int qnova_disasm_qbc(const char *path,const qnova_limits *limits,FILE*out,qnova_error *err){
    QbcProg p={0};if(load_qbc_file(path,limits,&p,err))return -1;
    fprintf(out,"QBC0 0.1 constants=%zu instructions=%zu entry=%u\n",p.consts.n,p.code.n,p.entry);
    for(size_t i=0;i<p.code.n;i++){
        QIns *in=&p.code.v[i];fprintf(out,"%04zu %-16s flags=%u",i,opcode_name(in->op),in->flags);
        for(size_t j=0;j<in->nops;j++) fprintf(out," %u",in->ops[j]);
        fputc('\n',out);
    }
    qbc_free(&p);return 0;
}
