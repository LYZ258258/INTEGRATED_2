// main.c
#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "token_reader.h"

void print_indent(int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
}

void print_expr(Expr* e, int indent) {
    if (!e) {
        print_indent(indent);
        printf("NULL\n");
        return;
    }
    print_indent(indent);
    switch (e->type) {
    case EXPR_ID:
        printf("ID: %s\n", e->id_name);
        break;
    case EXPR_NUMBER:
        printf("NUMBER: %d\n", e->number);
        break;
    case EXPR_BINARY_OP: {
        const char* op = "?";
        if (e->binop.op == T_AND) op = "AND";
        else if (e->binop.op == T_OR) op = "OR";
        else if (e->binop.op == T_XOR) op = "XOR";
        else if (e->binop.op == T_PLUS) op = "PLUS";
        else if (e->binop.op == T_MINUS) op = "MINUS";
        else if (e->binop.op == T_MUL) op = "MUL";
        else if (e->binop.op == T_DIV) op = "DIV";
        printf("BINARY_OP: %s (line %d)\n", op, e->line);
        print_expr(e->binop.left, indent + 1);
        print_expr(e->binop.right, indent + 1);
        break;
    }
    case EXPR_UNARY_OP:
        printf("UNARY_OP: op=%d\n", e->unop.op);
        print_expr(e->unop.operand, indent + 1);
        break;
    default:
        printf("UNKNOWN\n");
    }
}

void print_ast(Module* mod) {
    if (!mod) {
        printf("Parse failed.\n");
        return;
    }
    printf("=== Module: %s (line %d) ===\n", mod->name, mod->line);

    printf("\nPorts:\n");
    for (Port* p = mod->ports; p; p = p->next) {
        const char* dir = "INOUT";
        if (p->dir == PORT_INPUT) dir = "INPUT";
        else if (p->dir == PORT_OUTPUT) dir = "OUTPUT";
        printf("  - %s (%s) line %d\n", p->name, dir, p->line);
    }

    printf("\nWires:\n");
    for (VarDecl* v = mod->wires; v; v = v->next) {
        printf("  - %s (%s) line %d\n", v->name, v->is_reg ? "reg" : "wire", v->line);
    }

    printf("\nAssign statements:\n");
    for (AssignStmt* a = mod->assigns; a; a = a->next) {
        printf("  assign %s = \n", a->lhs);
        print_expr(a->rhs, 2);
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <verilog_file>\n", argv[0]);
        return 1;
    }

    // 步骤 1：调用verilog_lexer.exe 生成 token_output.txt
    char cmd[512];
    sprintf(cmd, "verilog_lexer.exe %s", argv[1]);
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Error: Failed to run verilog_lexer.exe\n");
        return 1;
    }

    // 步骤 2：读取 token_output.txt
    int count;
    Token* tokens = read_tokens_from_file("token_output.txt", &count);
    if (!tokens) {
        fprintf(stderr, "Failed to read tokens.\n");
        return 1;
    }

    printf("Tokens received: %d\n", count);

    // 步骤 3：解析
    Module* ast = parse(tokens, count);
    free(tokens);

    print_ast(ast);
    free_module(ast);

    return 0;
}