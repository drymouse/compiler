#include "mycc.h"

void generate(Node *node) {
    switch (node->kind) {
        case ND_NUM:
            printf("\tpush %d\n", node->val);
            return;
        case ND_LCV:
            gen_lcv(node);
            printf("\tpop rax\n");
            printf("\tmov rax, [rax]\n");
            printf("\tpush rax\n");
            return;
        case ND_ASN:
            gen_lcv(node->lhs);
            generate(node->rhs);
            break;
        default:
            generate(node->lhs);
            generate(node->rhs);
            break;
    }

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
        case ND_ASN:
            printf("\tmov [rax], rdi\n");
            printf("\tmov rax, rdi\n");
            break;
    }
    printf("\tpush rax\n");

}

void gen_lcv(Node *node) {
    if (node->kind != ND_LCV) {
        error("Invalid assignment");
    }
    printf("\tmov rax, rbp\n");
    printf("\tsub rax, %d\n", node->offset);
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
    } else if (node->kind == ND_LCV) {
        printf("%*sLocvar: %c\n", depth, "", node->offset / 8 + 0x60);
    } else {
        printf("%*s%d\n", depth, "", node->kind);
        nd_output(node->lhs, depth + 1);
        nd_output(node->rhs, depth + 1);
    }
}