// parser.c
#include "parser.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --------------------- 解析状态 --------------------- */
typedef struct {
    const Token* tokens;
    int count;
    int pos;            // 当前读取位置
    int error_flag;     // 是否已发生错误
} ParseState;

static void parse_error(ParseState* ps, const char* expected) {
    if (ps->error_flag) return;
    if (ps->pos >= ps->count) {
        fprintf(stderr, "Parse error at end of file: expected %s\n", expected);
    }
    else {
        Token cur = ps->tokens[ps->pos];
        fprintf(stderr, "Parse error at line %d, token '%s': expected %s\n",
            cur.line, cur.text, expected);
    }
    ps->error_flag = 1;
}

static Token peek(const ParseState* ps) {
    if (ps->pos >= ps->count) {
        Token eof = { T_EOF, "EOF", 0, 3 };
        return eof;
    }
    return ps->tokens[ps->pos];
}

static void consume(ParseState* ps) {
    if (ps->pos < ps->count) ps->pos++;
}

static int match(ParseState* ps, TokenType type) {
    if (ps->pos >= ps->count) return 0;
    if (ps->tokens[ps->pos].type == type) {
        consume(ps);
        return 1;
    }
    return 0;
}

static int expect(ParseState* ps, TokenType type, const char* type_name) {
    if (match(ps, type)) return 1;
    parse_error(ps, type_name);
    return 0;
}

/* --------------------- 前向声明 --------------------- */
static Expr* parse_expr(ParseState* ps);
static Expr* parse_term(ParseState* ps);
static Expr* parse_primary(ParseState* ps);
static Port* parse_ports(ParseState* ps);
static void parse_input_decl(ParseState* ps, Module* mod);
static void parse_output_decl(ParseState* ps, Module* mod);
static void parse_wire_decl(ParseState* ps, Module* mod);
static AssignStmt* parse_assign(ParseState* ps);
static Module* parse_module(ParseState* ps);

/* --------------------- 表达式解析 --------------------- */
/*
   expr   -> term ( '|' term )*
   term   -> primary ( '&' primary )*
   primary -> ID | NUMBER | '(' expr ')'
*/

static Expr* parse_expr(ParseState* ps) {
    Expr* left = parse_term(ps);
    while (peek(ps).type == T_OR) {
        int op = peek(ps).type;
        int line = peek(ps).line;
        consume(ps);  // 吃掉 '|'
        Expr* right = parse_term(ps);
        left = new_expr_binary(op, left, right, line);
    }
    return left;
}

static Expr* parse_term(ParseState* ps) {
    Expr* left = parse_primary(ps);
    while (peek(ps).type == T_AND) {
        int op = peek(ps).type;
        int line = peek(ps).line;
        consume(ps);  // 吃掉 '&'
        Expr* right = parse_primary(ps);
        left = new_expr_binary(op, left, right, line);
    }
    return left;
}

static Expr* parse_primary(ParseState* ps) {
    Token cur = peek(ps);
    if (cur.type == T_ID) {
        consume(ps);
        return new_expr_id(cur.text, cur.line);
    }
    else if (cur.type == T_NUMBER) {
        consume(ps);
        return new_expr_number(atoi(cur.text), cur.line);
    }
    else if (cur.type == T_LPAREN) {
        consume(ps);  // '('
        Expr* e = parse_expr(ps);
        expect(ps, T_RPAREN, "')'");
        return e;
    }
    else {
        parse_error(ps, "identifier, number, or '('");
        return new_expr_id("ERROR", cur.line);
    }
}

/* --------------------- 端口列表解析 --------------------- */
// ports -> '(' ID ( ',' ID )* ')'
static Port* parse_ports(ParseState* ps) {
    Port* head = NULL;
    Port* tail = NULL;

    if (!expect(ps, T_LPAREN, "'('"))
        return NULL;

    if (peek(ps).type == T_ID) {
        Token id = peek(ps);
        consume(ps);
        Port* p = new_port(id.text, PORT_INOUT, id.line);
        head = tail = p;
    }
    else {
        parse_error(ps, "port name");
        return NULL;
    }

    while (match(ps, T_COMMA)) {
        Token id = peek(ps);
        if (!expect(ps, T_ID, "port name")) break;
        Port* p = new_port(id.text, PORT_INOUT, id.line);
        tail->next = p;
        tail = p;
    }

    expect(ps, T_RPAREN, "')'");
    return head;
}

/* --------------------- 端口方向声明 --------------------- */
static void parse_input_decl(ParseState* ps, Module* mod) {
    consume(ps); // 'input'
    do {
        Token id = peek(ps);
        if (!expect(ps, T_ID, "port name")) break;
        for (Port* p = mod->ports; p; p = p->next) {
            if (strcmp(p->name, id.text) == 0) {
                p->dir = PORT_INPUT;
                break;
            }
        }
    } while (match(ps, T_COMMA));
    expect(ps, T_SEMICOLON, "';'");
}

static void parse_output_decl(ParseState* ps, Module* mod) {
    consume(ps); // 'output'
    do {
        Token id = peek(ps);
        if (!expect(ps, T_ID, "port name")) break;
        for (Port* p = mod->ports; p; p = p->next) {
            if (strcmp(p->name, id.text) == 0) {
                p->dir = PORT_OUTPUT;
                break;
            }
        }
    } while (match(ps, T_COMMA));
    expect(ps, T_SEMICOLON, "';'");
}

/* --------------------- wire 声明 --------------------- */
static void parse_wire_decl(ParseState* ps, Module* mod) {
    consume(ps); // 'wire'
    do {
        Token id = peek(ps);
        if (!expect(ps, T_ID, "wire name")) break;
        VarDecl* vd = new_vardecl(id.text, 0, id.line);
        vd->next = mod->wires;
        mod->wires = vd;
    } while (match(ps, T_COMMA));
    expect(ps, T_SEMICOLON, "';'");
}

/* --------------------- assign 语句 --------------------- */
static AssignStmt* parse_assign(ParseState* ps) {
    consume(ps); // 'assign'
    Token lhs = peek(ps);
    if (!expect(ps, T_ID, "left-hand side identifier")) return NULL;
    if (!expect(ps, T_ASSIGN_OP, "'='")) return NULL;
    Expr* rhs = parse_expr(ps);
    expect(ps, T_SEMICOLON, "';'");
    return new_assign(lhs.text, rhs, lhs.line);
}

/* --------------------- 顶层模块解析 --------------------- */
static Module* parse_module(ParseState* ps) {
    if (!expect(ps, T_MODULE, "'module'")) return NULL;

    Token name = peek(ps);
    if (!expect(ps, T_ID, "module name")) return NULL;

    Module* mod = new_module(name.text, name.line);

    mod->ports = parse_ports(ps);
    expect(ps, T_SEMICOLON, "';'");

    while (ps->pos < ps->count && peek(ps).type != T_ENDMODULE) {
        TokenType t = peek(ps).type;
        if (t == T_INPUT) {
            parse_input_decl(ps, mod);
        }
        else if (t == T_OUTPUT) {
            parse_output_decl(ps, mod);
        }
        else if (t == T_WIRE) {
            parse_wire_decl(ps, mod);
        }
        else if (t == T_REG) {
            parse_error(ps, "unsupported 'reg' (only wire supported)");
            break;
        }
        else if (t == T_ASSIGN) {
            AssignStmt* stmt = parse_assign(ps);
            if (stmt) {
                stmt->next = mod->assigns;
                mod->assigns = stmt;
            }
        }
        else {
            parse_error(ps, "declaration or statement");
            break;
        }
    }

    expect(ps, T_ENDMODULE, "'endmodule'");
    return mod;
}

/* --------------------- 对外接口 --------------------- */
Module* parse(const Token* tokens, int count) {
    ParseState ps = { tokens, count, 0, 0 };
    Module* mod = parse_module(&ps);
    if (ps.error_flag) {
        free_module(mod);
        return NULL;
    }
    return mod;
}

