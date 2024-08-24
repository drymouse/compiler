#include "mycc.h"

void push_reg(char *reg) {
    printf("\tpush %s\n", reg);
    stack_height += 8;
}

void push_val(int val) {
    printf("\tpush %d\n", val);
    stack_height += 8;
}

void pop(char *reg) {
    printf("\tpop %s\n", reg);
    stack_height -= 8;
}

void add_rsp(int val) {
    printf("\tadd rsp, 0x%x\n", val);
    stack_height -= val;
}

void sub_rsp(int val) {
    printf("\tsub rsp, 0x%x\n", val);
    stack_height += val;
}

void generate(Node *node) {
    switch (node->kind) {
        case ND_NUM:
            push_val(node->val);
            return;
        case ND_LCV:
            gen_lcv(node);
            pop("rax");
            printf("\tmov rax, [rax]\n");
            push_reg("rax");
            return;
        case ND_ASN:
            gen_lcv(node->lhs);
            generate(node->rhs);
            break;
        case ND_RET:
            generate(node->lhs);
            pop("rax");
            printf("\tleave\n");
            printf("\tret\n");
            return;
        case ND_IF_:
            generate(node->lhs);
            pop("rax");
            printf("\tcmp rax, 0\n");
            printf("\tje .Lelse%d\n", node->id);
            generate(node->rhs);
            pop("rax");
            printf(".Lelse%d:\n", node->id);
            if (node->third) {
                generate(node->third);
                pop("rax");
            }
            printf(".Lend%d:\n", node->id);
            push_reg("rax");
            return;
        case ND_WHL:
            printf(".Lbegin%d:\n", node->id);
            generate(node->lhs);
            pop("rax");
            printf("\tcmp rax, 0\n");
            printf("\tje .Lend%d\n", node->id);
            generate(node->rhs);
            pop("rax");
            printf("\tjmp .Lbegin%d\n", node->id);
            printf(".Lend%d:\n", node->id);
            push_reg("rax");
            return;
        case ND_FOR:
            if (node->lhs) {
                generate(node->lhs);
                pop("rax");
            }
            printf(".Lbegin%d:\n", node->id);
            if (node->rhs) {
                generate(node->rhs);
                pop("rax");
                printf("\tcmp rax, 0\n");
                printf("\tje .Lend%d\n", node->id);
            }
            generate(node->forth);
            pop("rax");
            if (node->third) {
                generate(node->third);
                pop("rax");
            }
            printf("\tjmp .Lbegin%d\n", node->id);
            printf(".Lend%d:\n", node->id);
            push_reg("rax");
            return;
        case ND_BLC:
            while (node->next) {
                generate(node->next);
                pop("rax");
                node = node->next;
            }
            push_reg("rax");
            return;
        case ND_FNC:
            Node *arg = node->fcall->args;
            int len = node->fcall->arglen;
            bool is_16aligned = len <= 6 && (stack_height / 8) % 2 == 1
                                || len >= 7 && (len + stack_height / 8) % 2 == 1;
            if (!is_16aligned) {
                push_reg("rbp");
            }
            for (int i = 0; i < len; i++) {
                if (!arg) {
                    error("arguments parse error!");
                }
                generate(arg);
                arg = arg->next;
            }
            if (len > 0) pop("rdi");
            if (len > 1) pop("rsi");
            if (len > 2) pop("rdx");
            if (len > 3) pop("rcx");
            if (len > 4) pop("r8");
            if (len > 5) pop("r9");
            printf("\tcall %s\n", node->fcall->name);
            (len > 6) ? add_rsp((len - 6) * 8) : 0;
            if (!is_16aligned) {
                pop("rbp");
            }
            push_reg("rax");
            return;
        case ND_DEF:
            printf("%s:\n", node->fdef->name);
            stack_height = 0;
            int arglen = node->fdef->arglen;
            int loclen = node->fdef->loclen;
            //prolog
            push_reg("rbp");
            printf("\tmov rbp, rsp\n");
            if (loclen > 0) {
                sub_rsp(loclen * 8);
            }
            if (arglen > 0) {
                printf("\tmov [rbp-0x8], rdi\n");
            }
            if (arglen > 1) {
                printf("\tmov [rbp-0x10], rsi\n");
            }
            if (arglen > 2) {
                printf("\tmov [rbp-0x8], rdi\n");
            }
            if (arglen > 3) {
                printf("\tmov [rbp-0x8], rdi\n");
            }
            if (arglen > 4) {
                printf("\tmov [rbp-0x8], rdi\n");
            }
            if (arglen > 5) {
                printf("\tmov [rbp-0x8], rdi\n");
            }
            if (arglen > 6) {
                printf("\tmov [rbp-0x8] rdi\n");
            }
            //main body
            while (node = node->next) {
                if (node->kind == ND_BLC) {
                    continue;
                }
                generate(node);
                pop("rax");
            }
            // epilog
            printf("\tleave\n");
            printf("\tret\n");
            return;
        case ND_ADR:
            gen_lcv(node->lhs);
            return;
        case ND_DRF:
            generate(node->lhs);
            pop("rax");
            printf("\tmov rax, [rax]\n");
            push_reg("rax");
            return;
        case ND_PAS:
            return;
        default:
            generate(node->lhs);
            generate(node->rhs);
            break;
    }

    pop("rdi");
    pop("rax");
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
    push_reg("rax");

}

void gen_lcv(Node *node) {
    if (node->kind != ND_LCV && node->kind != ND_DRF) {
        error("Invalid assignment");
    }
    if (node->kind == ND_LCV) {
        printf("\tmov rax, rbp\n");
        printf("\tsub rax, %d\n", node->offset);
        push_reg("rax");
    } else if (node->kind == ND_DRF) {
        gen_lcv(node->lhs);
        pop("rax");
        printf("\tmov rax, [rax]\n");
        push_reg("rax");
    }
    
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
    } else if (node->kind == ND_RET) {
        printf("%*sreturn\n", depth, "");
        nd_output(node->lhs, depth+1);
    } else if (node->kind == ND_BLC) {
        printf("%*s%d\n", depth, "", node->kind);
        while (node->next) {
            nd_output(node->next, depth + 1);
            node = node->next;
        }
    } else if (node->kind == ND_FNC) {
        printf("%*s%d\n", depth, "", node->kind);
    } else if (node->kind == ND_DEF) {
        while (node->next) {
            node = node->next;
            nd_output(node, depth+1);
        }
    } else {
        printf("%*s%d\n", depth, "", node->kind);
        (node->lhs) ? nd_output(node->lhs, depth + 1) : 0;
        (node->rhs) ? nd_output(node->rhs, depth + 1) : 0;
        (node->third) ? nd_output(node->third, depth + 1) : 0;
        (node->forth) ? nd_output(node->forth, depth + 1) : 0;
    }
}

void lv_output(Lvar *loc) {
    for (Lvar *var = loc; var; var = var->next) {
        printf("%.*s, %d, %d\n", var->len, var->name, var->len, var->offset);
    }
}

void fn_output(Fdef *fdef) {
    printf("function %s\n", fdef->name);
    for (Lvar *lvar = fdef->lvar; lvar; lvar = lvar->next) {
        printf("%.*s, %d, ", lvar->len, lvar->name, lvar->offset);
        Type *typ = lvar->type;
        while (typ->ptr_to) {
            printf("PTR -> ");
            typ = typ->ptr_to;
        }
        printf("INT\n");
    }
}