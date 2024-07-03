#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

typedef enum {
    TK_RESERVED,
    TK_IDENT,
    TK_RETURN,
    TK_IF,
    TK_ELSE,
    TK_WHILE,
    TK_FOR,
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
    ND_RET, // return
    ND_IF_, // if
    ND_ELS, // else
    ND_WHL, // while
    ND_FOR, // for
    ND_BLC, // block
    ND_NUM, // number
    ND_FNC, // function
} NodeKind;

typedef struct Token Token;
typedef struct Node Node;
typedef struct Lvar Lvar;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

struct Node {
    NodeKind kind;
    Node *lhs; // first
    Node *rhs; // second
    Node *third;
    Node *forth;
    int val;
    int offset;
    int id; // if / while / for
    Node *next; // for block
};

struct Lvar {
    Lvar *next;
    char *name;
    int len;
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
void lv_output(Lvar *loc);

extern Token *token;
extern Node *code[100];
extern Lvar *locals;
extern char *user_input;
extern int is_debugging;
extern int num_ctrl; // the number of if / while / for