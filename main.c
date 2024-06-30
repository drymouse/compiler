#include "mycc.h"

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