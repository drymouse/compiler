#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

typedef enum {
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
} NodeKind;

typedef struct Token Token;
typedef struct Node Node;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
};

struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
};

Node *expr();
Node *mul();
Node *primary();
Node *unary();

Token *token;
char *user_input;

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s^ ", pos, " ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) {
        return false;
    } else {
        token = token->next;
        return true;
    }
}

void expect(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) {
        error_at(token->str, "expected %c, but actually %c", op, token->str[0]);
    } else {
        token = token->next;
    }
}

int expect_number() {
    if (token->kind != TK_NUM) {
        error_at(token->str, "not a number");
    } else {
        int val = token->val;
        token = token->next;
        return val;
    }
}

bool at_eof() {
    return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *nod = calloc(1, sizeof(Node));
    nod->kind = kind;
    nod->lhs = lhs;
    nod->rhs = rhs;
    return nod;
}

Node *new_node_num(int val) {
    Node *nod = calloc(1, sizeof(Node));
    nod->kind = ND_NUM;
    nod->val = val;
    return nod;
}

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
            cur = new_token(TK_RESERVED, cur, p);
            p++;
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(token->str, "Tokenize failed...");
    }
    new_token(TK_EOF, cur, p);
    return head.next;
}

Node *expr() {
    Node *node = mul();

    for (;;) {
        if (consume('+')) {
            node = new_node(ND_ADD, node, mul());
        } else if (consume('-')) {
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume('*')) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume('/')) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

Node *unary() {
    if (consume('+')) {
        return primary();
    }
    if (consume('-')) {
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    return primary();
}

Node *primary() {
    if (consume('(')) {
        Node *node = expr();
        expect(')');
        return node;
    } else {
        Node *node = new_node_num(expect_number());
        return node;
    }
}

void generate(Node *node) {
    if (node->kind == ND_NUM) {
        printf("\tpush %d\n", node->val);
        return;
    }
    generate(node->lhs);
    generate(node->rhs);

    printf("\tpop rdi\n");
    printf("\tpop rax\n");
    switch (node->kind) {
        case ND_ADD:
            printf("\tadd rax, rdi\n");
            break;
        case ND_SUB:
            printf("\tsub rax, rdi\n");
            break;
        case ND_MUL:
            printf("\timul rax, rdi\n");
            break;
        case ND_DIV:
            printf("\tcqo\n");
            printf("\tidiv rdi\n");
            break;
    }
    printf("\tpush rax\n");

}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Invalid arguments\n");
    }

    // Tokenize
    user_input = argv[1];
    token = tokenize(user_input);
    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    generate(node);
    printf("\tpop rax\n");
    printf("\tret\n");
    return 0;
}