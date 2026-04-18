// lexer.h
#ifndef LEXER_H
#define LEXER_H

typedef enum {
    T_MODULE, T_INPUT, T_OUTPUT, T_WIRE, T_REG, T_ASSIGN,
    T_ALWAYS, T_POSEDGE, T_NEGEDGE, T_ENDMODULE,
    T_ID, T_NUMBER,
    T_ASSIGN_OP,
    T_AND, T_OR, T_XOR,
    T_PLUS, T_MINUS, T_MUL, T_DIV,
    T_LPAREN, T_RPAREN,
    T_LBRACE, T_RBRACE,
    T_COMMA, T_SEMICOLON, T_DOT,
    T_EOF    // ±ãÓÚ Parser ÅÐ¶Ï½áÊø
} TokenType;

typedef struct Token {
    TokenType type;
    char text[256];
    int line;
    int length;
} Token;

#endif