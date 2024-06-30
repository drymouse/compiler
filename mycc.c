#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define STR(var) #var

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
    ND_EQU, // EQUAL
    ND_NEQ, // NOT EQUAL
    ND_GTR, // GREATER
    ND_GTE, // GREATER EQUAL
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
};

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *primary();
Node *unary();

Token *token;
char *user_input;
int is_debugging;

void error_at(char *loc, char *fmt, ...) {
    if (is_debugging) {
        printf("error\n");
    }
    va_list ap;
    va_start(ap, fmt);
    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s^ ", pos, " ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char *op) {
    if (token->kind != TK_RESERVED || token->len != strlen(op) || strncmp(token->str, op, token->len)) {
        return false;
    } else {
        token = token->next;
        return true;
    }
}

void expect(char *op) {
    if (token->kind != TK_RESERVED || token->len != strlen(op) || strncmp(token->str, op, token->len)) {
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

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    tok->next = NULL;
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

        if ((strncmp(p, "==", 2) && strncmp(p, "!=", 2) && strncmp(p, "<=", 2) && strncmp(p, ">=", 2)) == 0) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p+=2;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '>' || *p == '<') {
            cur = new_token(TK_RESERVED, cur, p, 1);
            p++;
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 10);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(token->str, "Tokenize failed...");
    }
    new_token(TK_EOF, cur, p, 1);
    return head.next;
}

Node *expr() {
    return equality();
}

Node *equality() {
    Node *node = relational();

    for (;;) {
        if (consume("==")) {
            node = new_node(ND_EQU, node, relational());
        } else if (consume("!=")) {
            node = new_node(ND_NEQ, node, relational());
        } else {
            return node;
        }
    }
}

Node *relational() {
    Node *node = add();

    for (;;) {
        if (consume(">=")) {
            node = new_node(ND_GTE, node, add());
        } else if (consume("<=")) {
            node = new_node(ND_GTE, add(), node);
        } else if (consume(">")) {
            node = new_node(ND_GTR, node, add());
        } else if (consume("<")) {
            node = new_node(ND_GTR, add(), node);
        } else {
            return node;
        }
    }
}

Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume("+")) {
            node = new_node(ND_ADD, node, mul());
        } else if (consume("-")) {
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume("/")) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

Node *unary() {
    if (consume("+")) {
        return primary();
    }
    if (consume("-")) {
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    return primary();
}

Node *primary() {
    if (consume("(")) {
        Node *node = expr();
        expect(")");
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
        case ND_EQU:
            printf("\tcmp rax, rdi\n");
            printf("\tsete al\n");
            printf("\tmovzb rax, al\n");
            break;
        case ND_NEQ:
            printf("\tcmp rax, rdi\n");
            printf("\tsetne al\n");
            printf("\tmovzb rax, al\n");
            break;
        case ND_GTR:
            printf("\tcmp rax, rdi\n");
            printf("\tsetg al\n");
            printf("\tmovzb rax, al\n");
            break;
        case ND_GTE:
            printf("\tcmp rax, rdi\n");
            printf("\tsetge al\n");
            printf("\tmovzb rax, al\n");
            break;
    }
    printf("\tpush rax\n");

}

void tk_output(Token *tok) {
    while (tok) {
        if (tok->kind == TK_NUM) {
            printf("(%d) ", tok->val);
        } else {
            printf("(%.*s) ", tok->len, tok->str);
        }
        tok = tok->next;
    }
    printf("\n");
}

void nd_output(Node *node, int depth) {
    if (node->kind == ND_NUM) {
        printf("%*sNumber: %d\n", depth, "", node->val);
    } else {
        printf("%*s%d\n", depth, "", node->kind);
        nd_output(node->lhs, depth + 1);
        nd_output(node->rhs, depth + 1);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Invalid arguments\n");
    } else if (argc == 2) {
        is_debugging = 0;
    } else {
        is_debugging = atoi(argv[2]);
    }

    // Tokenize
    user_input = argv[1];
    token = tokenize(user_input);
    if (is_debugging) {
        tk_output(token);
    }
    Node *node = expr();
    if (is_debugging) {
        nd_output(node, 0);
    }

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    generate(node);
    printf("\tpop rax\n");
    printf("\tret\n");
    return 0;
}