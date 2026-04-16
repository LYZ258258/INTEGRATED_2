%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylineno = 1;
FILE* token_file;

typedef enum {
    T_MODULE, T_INPUT, T_OUTPUT, T_WIRE, T_REG, T_ASSIGN,
    T_ALWAYS, T_POSEDGE, T_NEGEDGE, T_ENDMODULE,
    T_ID, T_NUMBER,
    T_ASSIGN_OP,
    T_AND, T_OR, T_XOR,
    T_PLUS, T_MINUS, T_MUL, T_DIV,
    T_LPAREN, T_RPAREN,
    T_LBRACE, T_RBRACE,
    T_COMMA, T_SEMICOLON, T_DOT
} TokenType;

typedef struct Token {
    TokenType type;
    char text[256];
    int line;
    int length;
} Token;

Token current_token;

void save_token(TokenType type, const char* text) {
    current_token.type = type;
    strcpy(current_token.text, text);
    current_token.line = yylineno;
    current_token.length = strlen(text);

    const char* type_name;
    switch(type) {
        case T_MODULE: type_name = "MODULE"; break;
        case T_INPUT: type_name = "INPUT"; break;
        case T_OUTPUT: type_name = "OUTPUT"; break;
        case T_WIRE: type_name = "WIRE"; break;
        case T_REG: type_name = "REG"; break;
        case T_ASSIGN: type_name = "ASSIGN"; break;
        case T_ENDMODULE: type_name = "ENDMODULE"; break;
        case T_ID: type_name = "IDENTIFIER"; break;
        case T_NUMBER: type_name = "NUMBER"; break;
        case T_ASSIGN_OP: type_name = "ASSIGN_OP"; break;
        case T_AND: type_name = "AND"; break;
        case T_OR: type_name = "OR"; break;
        case T_XOR: type_name = "XOR"; break;
        case T_LPAREN: type_name = "LPAREN"; break;
        case T_RPAREN: type_name = "RPAREN"; break;
        case T_LBRACE: type_name = "LBRACE"; break;
        case T_RBRACE: type_name = "RBRACE"; break;
        case T_COMMA: type_name = "COMMA"; break;
        case T_SEMICOLON: type_name = "SEMICOLON"; break;
        case T_DOT: type_name = "DOT"; break;
        default: type_name = "UNKNOWN"; break;
    }
    fprintf(token_file, "[Line %-3d] TYPE=%-18s TEXT=%s\n",
            current_token.line, type_name, current_token.text);
}
%}

ID          [a-zA-Z_$][a-zA-Z0-9_$]*
DIGIT       [0-9]
NUMBER      [0-9]+['"]*[bBdDhHoO][0-9xXzZa-fA-F]+

%%

module      { save_token(T_MODULE, yytext); }
input       { save_token(T_INPUT, yytext); }
output      { save_token(T_OUTPUT, yytext); }
wire        { save_token(T_WIRE, yytext); }
reg         { save_token(T_REG, yytext); }
assign      { save_token(T_ASSIGN, yytext); }
always      { save_token(T_ALWAYS, yytext); }
posedge     { save_token(T_POSEDGE, yytext); }
negedge     { save_token(T_NEGEDGE, yytext); }
endmodule   { save_token(T_ENDMODULE, yytext); }

{ID}        { save_token(T_ID, yytext); }
{NUMBER}    { save_token(T_NUMBER, yytext); }

=           { save_token(T_ASSIGN_OP, yytext); }
&           { save_token(T_AND, yytext); }
\|          { save_token(T_OR, yytext); }
\^          { save_token(T_XOR, yytext); }
\+          { save_token(T_PLUS, yytext); }
\-          { save_token(T_MINUS, yytext); }
\*          { save_token(T_MUL, yytext); }
\/          { save_token(T_DIV, yytext); }

\(          { save_token(T_LPAREN, yytext); }
\)          { save_token(T_RPAREN, yytext); }
\{          { save_token(T_LBRACE, yytext); }
\}          { save_token(T_RBRACE, yytext); }
,           { save_token(T_COMMA, yytext); }
;           { save_token(T_SEMICOLON, yytext); }
\.          { save_token(T_DOT, yytext); }

\/\/.*      ;
\/\*[^*]*\*\/ ;

[ \t\r]     ;
\n          { yylineno++; }
.           { }

%%

int yywrap() { return 1; }

int main(int argc, char** argv) {
    token_file = fopen("token_output.txt", "w");
    if (!token_file) { perror("fopen failed"); return 1; }

    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) { perror("fopen failed"); fclose(token_file); return 1; }
    }

    yylex();  // 直接全文件扫描，不中断

    fclose(token_file);
    if (yyin) fclose(yyin);
    printf("Tokens saved to token_output.txt\n");
    return 0;
}