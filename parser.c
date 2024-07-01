#include "mycc.h"

void error(char *msg) {
    printf("%s\n", msg);
}

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

Token *consume_ident() {
    if (token->kind == TK_IDENT) {
        Token *var = token;
        token = token->next;
        return var;
    } else {
        return NULL;
    }
}

Lvar *find_lvar(Token *tok) {
    for (Lvar *var = locals; var; var = var->next) {
        if (var->len == tok->len && !strncmp(var->name, tok->str, var->len)) {
            return var;
        }
    }
    return NULL;
}

void expect(char *op) {
    if (token->kind != TK_RESERVED || token->len != strlen(op) || strncmp(token->str, op, token->len)) {
        error_at(token->str, "expected %s, but actually %s", op, token->str);
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

bool is_alpnum(char c) {
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
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

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '>' || *p == '<'
            || *p == '=' || *p == ';') {
            cur = new_token(TK_RESERVED, cur, p, 1);
            p++;
            continue;
        }

        if (!strncmp(p, "return", 6) && !is_alpnum(p[6])) {
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
            continue;
        }

        if (!strncmp(p, "if", 2) && !is_alpnum(p[2])) {
            cur = new_token(TK_IF, cur, p, 2);
            p += 2;
            continue;
        }

        if (!strncmp(p, "else", 4) && !is_alpnum(p[4])) {
            cur = new_token(TK_ELSE, cur, p, 4);
            p += 4;
            continue;
        }

        if (!strncmp(p, "while", 5) && !is_alpnum(p[5])) {
            cur = new_token(TK_WHILE, cur, p, 5);
            p += 5;
            continue;
        }

        if (!strncmp(p, "for", 3) && !is_alpnum(p[3])) {
            cur = new_token(TK_FOR, cur, p, 3);
            p += 3;
            continue;
        }

        if (('a' <= *p && *p <= 'z') || *p == '_') {
            int len = 1;
            char *start = p;
            p++;
            while (('a' <= *p && *p <= 'z') || *p == '_') {
                len++;
                p++;
            }
            cur = new_token(TK_IDENT, cur, start, len);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 10);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "Tokenize failed...");
    }
    new_token(TK_EOF, cur, p, 1);
    return head.next;
}

void program() {
    int i = 0;
    while (!at_eof()) {
        code[i] = stmt();
        i++;
    }
    code[i] = NULL;
}

Node *stmt() {
    Node *node;
    switch (token->kind) {
        case TK_RETURN:
            token = token->next;
            node = calloc(1, sizeof(Node));
            node->kind = ND_RET;
            node->lhs = expr();
            expect(";");
            break;
        case TK_IF:
            token = token->next;
            node = calloc(1, sizeof(Node));
            node->kind = ND_IF_;
            expect("(");
            node->lhs = expr();
            expect(")");
            node->rhs = stmt();
            if (consume("else")) {
                node->third = stmt();
            }
            break;
        case TK_WHILE:
            token = token->next;
            node = calloc(1, sizeof(Node));
            node->kind = ND_WHL;
            expect("(");
            node->lhs = expr();
            expect(")");
            node->rhs = stmt();
            break;
        case TK_FOR:
            token = token->next;
            node = calloc(1, sizeof(Node));
            node->kind = ND_FOR;
            expect("(");
            if (consume(";")) {
                node->lhs = NULL;
            } else {
                node->lhs = expr();
                expect(";");
            }
            if (consume(";")) {
                node->rhs = NULL;
            } else {
                node->rhs = expr();
                expect(";");
            }
            if (consume(")")) {
                node->lhs = NULL;
            } else {
                node->lhs = expr();
                expect(")");
            }
            node->forth = stmt();
            break;

        default:
            node = expr();
            expect(";");
    }
    return node;
}

Node *expr() {
    return assign();
}

Node *assign() {
    Node *node = equality();

    if (consume("=")) {
        node = new_node(ND_ASN, node, assign());
    }
    return node;
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
    Token *tok;
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    } else if (tok = consume_ident()) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LCV;

        Lvar *lvar = find_lvar(tok);
        if (lvar) {
            node->offset = lvar->offset;
        } else {
            Lvar *new_lvar = calloc(1, sizeof(Lvar));
            new_lvar->next = locals;
            new_lvar->name = tok->str;
            new_lvar->len = tok->len;
            new_lvar->offset = (locals) ? locals->offset + 8 : 8;
            locals = new_lvar;
            node->offset = new_lvar->offset;
        }
        return node;
    } else {
        Node *node = new_node_num(expect_number());
        return node;
    }
}