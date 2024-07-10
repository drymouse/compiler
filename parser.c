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
    char *line_begin = user_input;
    char *line_end = user_input + strlen(user_input);
    int line = 1;
    bool met_loc = false;
    for (char *cur = user_input; ; cur++) {
        if (cur == loc) {
            met_loc = true;
        }
        if (*cur == '\n') {
            if (met_loc) {
                line_end = cur - 1;
                break;
            }
            line_begin = cur + 1;
            line++;
        }
    }
    int pos = loc - line_begin;
    int mojisuu;
    fprintf(stderr, "line %d: %n", line, &mojisuu);
    fprintf(stderr, "%.*s\n", (int)(line_end - line_begin), line_begin);
    fprintf(stderr, "%*s^ ", pos + mojisuu, " ");
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

bool consume_type(TokenKind kind) {
    if (token->kind != kind) {
        return false;
    } else {
        token = token->next;
        return true;
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

Token *expect_ident() {
    if (token->kind != TK_IDENT) {
        error_at(token->str, "not a identifier");
    } else {
        Token *val = token;
        token = token->next;
        return val;
    }
}

void expect_type() {
    if (token->kind != TK_TYPE) {
        error_at(token->str, "not a type");
    } else {
        token = token->next;
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
            || *p == '=' || *p == ';' || *p == '{' || *p == '}' || *p == ',' || *p == '*' || *p == '&') {
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

        if (!strncmp(p, "int", 3) && !is_alpnum(p[3])) {
            cur = new_token(TK_TYPE, cur, p, 3);
            p += 3;
            continue;
        }

        if (('a' <= *p && *p <= 'z') 
         || ('A' <= *p && *p <= 'Z') 
         || *p == '_') {
            int len = 1;
            char *start = p;
            p++;
            while (is_alpnum(*p)) {
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

char *make_string(char *source, int len) {
    char *strings = calloc(1, len + 1);
    memcpy(strings, source, len);
    *(strings+len) = 0;
    return strings;
}

void program() {
    int i = 0;
    while (!at_eof()) {
        code[i] = definition();
        i++;
    }
    code[i] = NULL;
}

Node *definition() {
    Node *node = calloc(1, sizeof(Node));
    Fdef *fdef = calloc(1, sizeof(Fdef));
    node->kind = ND_DEF;
    int arglen = 0;
    expect_type();
    Token *tok = expect_ident();
    fdef->name = make_string(tok->str, tok->len);
    expect("(");
    for (arglen = 0; !consume(")"); arglen++) {
        if (arglen) {
            expect(",");
        }
        expect_type();
        Token *argtok = expect_ident();
        Lvar *arg = calloc(1, sizeof(Node));
        arg->name = argtok->str;
        arg->len = argtok->len;
        arg->offset = (fdef->lvar) ? fdef->lvar->offset + 8 : 8;
        arg->next = fdef->lvar;
        fdef->lvar = arg;
    }
    expect("{");
    locals = fdef->lvar;
    Node *last = node;
    while (!consume("}")) {
        Node *tmp = stmt();
        last->next = tmp;
        last = tmp;
    }
    
    fdef->lvar = locals;
    fdef->arglen = arglen;
    fdef->loclen = (fdef->lvar) ? fdef->lvar->offset / 8 : 0;
    node->fdef = fdef;
    return node;
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
            node->id = num_ctrl;
            num_ctrl++;
            expect("(");
            node->lhs = expr();
            expect(")");
            node->rhs = stmt();
            if (consume_type(TK_ELSE)) {
                node->third = stmt();
            }
            break;
        case TK_WHILE:
            token = token->next;
            node = calloc(1, sizeof(Node));
            node->kind = ND_WHL;
            node->id = num_ctrl;
            num_ctrl++;
            expect("(");
            node->lhs = expr();
            expect(")");
            node->rhs = stmt();
            break;
        case TK_FOR:
            token = token->next;
            node = calloc(1, sizeof(Node));
            node->kind = ND_FOR;
            node->id = num_ctrl;
            num_ctrl++;
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
                node->third = NULL;
            } else {
                node->third = expr();
                expect(")");
            }
            node->forth = stmt();
            break;
        case TK_TYPE:
            token = token->next;
            Token *tok = expect_ident();
            Lvar *lvar = find_lvar(tok);
            node = calloc(1, sizeof(Node));
            node->kind = ND_LCV;
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
            expect(";");
            break;
        default:
            if (consume("{")) {
                node = calloc(1, sizeof(Node));
                node->kind = ND_BLC;
                Node *last = node;
                while (!consume("}")) {
                    last->next = stmt();
                    last = last->next;
                }
                last->next = NULL;
            } else {
                node = expr();
                expect(";");
            }
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
    if (consume("*")) {
        return new_node(ND_DRF, unary(), NULL);
    }
    if (consume("&")) {
        return new_node(ND_ADR, unary(), NULL);
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
        if (consume("(")) {
            Node *node = calloc(1, sizeof(Node));
            Fcall *fcall = calloc(1, sizeof(Fcall));
            node->kind = ND_FNC;
            // node->lhs = tok->str;
            // node->rhs = tok->len;
            int len;
            Node *arg = NULL;
            for (len = 0; !consume(")"); len++) {
                if (len) expect(",");
                Node *tmp = expr();
                tmp->next = arg; // NULL <- arg1 <- arg2 <- arg3
                arg = tmp;
            }
            fcall->arglen = len;
            fcall->args = arg;
            fcall->name = make_string(tok->str, tok->len);
            (is_debugging) ? printf("fcall: %s\n" ,fcall->name) : 0;
            node->fcall = fcall;
            return node;
        }
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LCV;

        Lvar *lvar = find_lvar(tok);
        if (lvar) {
            node->offset = lvar->offset;
        } else {
            error_at(tok->str, "%.*s: Not a defined variable", tok->len, tok->str);
            /*
            Lvar *new_lvar = calloc(1, sizeof(Lvar));
            new_lvar->next = locals;
            new_lvar->name = tok->str;
            new_lvar->len = tok->len;
            new_lvar->offset = (locals) ? locals->offset + 8 : 8;
            locals = new_lvar;
            node->offset = new_lvar->offset;
            */
        }
        return node;
    } else {
        Node *node = new_node_num(expect_number());
        return node;
    }
}