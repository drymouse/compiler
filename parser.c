#include "mycc.h"

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