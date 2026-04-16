// ast.h
#ifndef AST_H
#define AST_H

#include <stddef.h>

/* --------------------- 表达式节点 --------------------- */
typedef enum {
    EXPR_ID,         // 变量名
    EXPR_NUMBER,     // 数字常量
    EXPR_BINARY_OP,  // 双目运算，如 a & b
    EXPR_UNARY_OP    // 单目运算（预留）
} ExprType;

typedef struct Expr {
    ExprType type;
    union {
        char id_name[64];               // EXPR_ID
        int number;                     // EXPR_NUMBER
        struct {
            int op;                     // TokenType，如 T_AND, T_OR
            struct Expr* left;
            struct Expr* right;
        } binop;
        struct {
            int op;                     // TokenType，如 '!', '~'
            struct Expr* operand;
        } unop;
    };
    int line;           // 源文件行号，用于报错
} Expr;

/* --------------------- 端口方向 --------------------- */
typedef enum {
    PORT_INPUT,
    PORT_OUTPUT,
    PORT_INOUT   // 预留
} PortDir;

/* --------------------- 端口节点（链表） --------------------- */
typedef struct Port {
    char name[64];
    PortDir dir;
    int line;
    struct Port* next;
} Port;

/* --------------------- 变量声明（wire/reg）链表 --------------------- */
typedef struct VarDecl {
    char name[64];
    int is_reg;      // 0: wire, 1: reg
    int line;
    struct VarDecl* next;
} VarDecl;

/* --------------------- assign 语句链表 --------------------- */
typedef struct AssignStmt {
    char lhs[64];     // 左侧变量名
    Expr* rhs;        // 右侧表达式树
    int line;
    struct AssignStmt* next;
} AssignStmt;

/* --------------------- 顶层模块节点 --------------------- */
typedef struct Module {
    char name[64];
    Port* ports;           // 端口链表头
    VarDecl* wires;        // wire/reg 声明链表头
    AssignStmt* assigns;   // assign 语句链表头
    int line;
} Module;

/* --------------------- 工厂函数 --------------------- */
Expr* new_expr_id(const char* name, int line);
Expr* new_expr_number(int value, int line);
Expr* new_expr_binary(int op, Expr* left, Expr* right, int line);
Expr* new_expr_unary(int op, Expr* operand, int line);

Port* new_port(const char* name, PortDir dir, int line);
VarDecl* new_vardecl(const char* name, int is_reg, int line);
AssignStmt* new_assign(const char* lhs, Expr* rhs, int line);
Module* new_module(const char* name, int line);

/* --------------------- 内存释放 --------------------- */
void free_expr(Expr* e);
void free_ports(Port* p);
void free_vardecls(VarDecl* v);
void free_assigns(AssignStmt* a);
void free_module(Module* mod);

#endif