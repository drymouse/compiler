#include "mycc.h"

Token *token;
Node *code[100];
Lvar *locals;
char *user_input;
int is_debugging;

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
        printf("locals is %s.\n", (!locals) ? "NULL" : "not NULL");
        tk_output(token);
    }
    program();
    if (is_debugging) {
        for (int i = 0; code[i]; i++) {
            nd_output(code[i], 0);
        }
        lv_output(locals);
    }

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    // prolog
    printf("\tpush rbp\n");
    printf("\tmov rbp, rsp\n");
    printf("\tsub rsp, 0x130\n");
    // main body
    for (int i = 0; code[i]; i++) {
        generate(code[i]);
        printf("\tpop rax\n");
    }
    // epilog
    printf("\tleave\n");
    printf("\tret\n");
    return 0;
}