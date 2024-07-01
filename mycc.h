#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

typedef enum {
    TK_RESERVED,
    TK_IDENT,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_EQU, // EQUAL
    ND_NEQ, // NOT EQUAL
    ND_GTR, // GREATER
    ND_GTE, // GREATER EQUAL
    ND_ASN, // assign
    ND_LCV, // local variable
    ND_NUM,
} NodeKind;

typedef struct Token Token;
typedef struct Node Node;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
    int offset;
};

void error(char *msg);
void error_at(char *loc, char *fmt, ...);
Token *tokenize(char *p);

void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *primary();
Node *unary();

void gen_lcv(Node *node);
void generate(Node *node);
void tk_output(Token *tok);
void nd_output(Node *node, int depth);

extern Token *token;
extern char *user_input;
extern int is_debugging;
extern Node *code[100];