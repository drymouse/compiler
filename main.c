#include "mycc.h"

Token *token;
Node *code[100];
Lvar *locals;
char *user_input;
int is_debugging;
int num_ctrl;
int stack_height;

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Invalid arguments\n");
    }

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-f")) {
            char *filename = argv[i + 1];
            FILE *fd = fopen(filename, "r");
            user_input = malloc(0x1000);
            char *line = user_input;
            while (fgets(line, 0x1000, fd)) {
                line = line + strlen(line);
            }
            i++;
        } else if (!strcmp(argv[i], "-c")) {
            user_input = argv[i + 1];
            i++;
        } else if (!strcmp(argv[i], "-d")) {
            is_debugging = 1;
        }
    }

    // Tokenize
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
        for (int i = 0; code[i]; i++) {
            fn_output(code[i]->fdef);
        }
    }

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    // prolog
    // main body
    for (int i = 0; code[i]; i++) {
        generate(code[i]);
    }
    // epilog
    return 0;
}