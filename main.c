#include "mycc.h"

Token *token;
Node *code[100];
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
        tk_output(token);
    }
    program();
    if (is_debugging) {
        nd_output(code[0], 0);
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