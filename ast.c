#include "ast.h"
#include <stdlib.h>
#include <string.h>


/* --------------------- 工厂函数实现 --------------------- */
Expr* new_expr_id(const char* name, int line) {
    Expr* e = (Expr*)malloc(sizeof(Expr));
    e->type = EXPR_ID;
    strncpy(e->id_name, name, 63);
    e->id_name[63] = '\0';
    e->line = line;
    return e;
}

Expr* new_expr_number(int value, int line) {
    Expr* e = (Expr*)malloc(sizeof(Expr));
    e->type = EXPR_NUMBER;
    e->number = value;
    e->line = line;
    return e;
}

Expr* new_expr_binary(int op, Expr* left, Expr* right, int line) {
    Expr* e = (Expr*)malloc(sizeof(Expr));
    e->type = EXPR_BINARY_OP;
    e->binop.op = op;
    e->binop.left = left;
    e->binop.right = right;
    e->line = line;
    return e;
}

Expr* new_expr_unary(int op, Expr* operand, int line) {
    Expr* e = (Expr*)malloc(sizeof(Expr));
    e->type = EXPR_UNARY_OP;
    e->unop.op = op;
    e->unop.operand = operand;
    e->line = line;
    return e;
}

Port* new_port(const char* name, PortDir dir, int line) {
    Port* p = (Port*)malloc(sizeof(Port));
    strncpy(p->name, name, 63);
    p->name[63] = '\0';
    p->dir = dir;
    p->line = line;
    p->next = NULL;
    return p;
}

VarDecl* new_vardecl(const char* name, int is_reg, int line) {
    VarDecl* v = (VarDecl*)malloc(sizeof(VarDecl));
    strncpy(v->name, name, 63);
    v->name[63] = '\0';
    v->is_reg = is_reg;
    v->line = line;
    v->next = NULL;
    return v;
}

AssignStmt* new_assign(const char* lhs, Expr* rhs, int line) {
    AssignStmt* a = (AssignStmt*)malloc(sizeof(AssignStmt));
    strncpy(a->lhs, lhs, 63);
    a->lhs[63] = '\0';
    a->rhs = rhs;
    a->line = line;
    a->next = NULL;
    return a;
}

Module* new_module(const char* name, int line) {
    Module* m = (Module*)malloc(sizeof(Module));
    strncpy(m->name, name, 63);
    m->name[63] = '\0';
    m->ports = NULL;
    m->wires = NULL;
    m->assigns = NULL;
    m->line = line;
    return m;
}

/* --------------------- 内存释放函数 --------------------- */
void free_expr(Expr* e) {
    if (!e) return;
    if (e->type == EXPR_BINARY_OP) {
        free_expr(e->binop.left);
        free_expr(e->binop.right);
    }
    else if (e->type == EXPR_UNARY_OP) {
        free_expr(e->unop.operand);
    }
    free(e);
}

void free_ports(Port* p) {
    while (p) {
        Port* next = p->next;
        free(p);
        p = next;
    }
}

void free_vardecls(VarDecl* v) {
    while (v) {
        VarDecl* next = v->next;
        free(v);
        v = next;
    }
}

void free_assigns(AssignStmt* a) {
    while (a) {
        AssignStmt* next = a->next;
        free_expr(a->rhs);
        free(a);
        a = next;
    }
}

void free_module(Module* mod) {
    if (!mod) return;
    free_ports(mod->ports);
    free_vardecls(mod->wires);
    free_assigns(mod->assigns);
    free(mod);
}