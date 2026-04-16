// token_reader.c
#include "token_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static TokenType parse_type_string(const char* str) {
    if (strcmp(str, "MODULE") == 0) return T_MODULE;
    if (strcmp(str, "INPUT") == 0) return T_INPUT;
    if (strcmp(str, "OUTPUT") == 0) return T_OUTPUT;
    if (strcmp(str, "WIRE") == 0) return T_WIRE;
    if (strcmp(str, "REG") == 0) return T_REG;
    if (strcmp(str, "ASSIGN") == 0) return T_ASSIGN;
    if (strcmp(str, "ALWAYS") == 0) return T_ALWAYS;
    if (strcmp(str, "POSEDGE") == 0) return T_POSEDGE;
    if (strcmp(str, "NEGEDGE") == 0) return T_NEGEDGE;
    if (strcmp(str, "ENDMODULE") == 0) return T_ENDMODULE;
    if (strcmp(str, "IDENTIFIER") == 0) return T_ID;
    if (strcmp(str, "NUMBER") == 0) return T_NUMBER;
    if (strcmp(str, "ASSIGN_OP") == 0) return T_ASSIGN_OP;
    if (strcmp(str, "AND") == 0) return T_AND;
    if (strcmp(str, "OR") == 0) return T_OR;
    if (strcmp(str, "XOR") == 0) return T_XOR;
    if (strcmp(str, "PLUS") == 0) return T_PLUS;
    if (strcmp(str, "MINUS") == 0) return T_MINUS;
    if (strcmp(str, "MUL") == 0) return T_MUL;
    if (strcmp(str, "DIV") == 0) return T_DIV;
    if (strcmp(str, "LPAREN") == 0) return T_LPAREN;
    if (strcmp(str, "RPAREN") == 0) return T_RPAREN;
    if (strcmp(str, "LBRACE") == 0) return T_LBRACE;
    if (strcmp(str, "RBRACE") == 0) return T_RBRACE;
    if (strcmp(str, "COMMA") == 0) return T_COMMA;
    if (strcmp(str, "SEMICOLON") == 0) return T_SEMICOLON;
    if (strcmp(str, "DOT") == 0) return T_DOT;
    return T_EOF;
}

Token* read_tokens_from_file(const char* filename, int* out_count) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror("Failed to open token file");
        *out_count = 0;
        return NULL;
    }

    int capacity = 256;
    Token* tokens = (Token*)malloc(capacity * sizeof(Token));
    int count = 0;

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        int line_num;
        char type_str[32];
        char text[256];

        // 格式: [Line 1  ] TYPE=MODULE             TEXT=module
        if (sscanf(line, "[Line %d ] TYPE=%s TEXT=%s", &line_num, type_str, text) != 3) {
            // 尝试更宽松的格式
            if (sscanf(line, "[Line %d ] %*[ ]TYPE=%s %*[ ]TEXT=%s", &line_num, type_str, text) != 3) {
                fprintf(stderr, "Warning: Skipping malformed line: %s", line);
                continue;
            }
        }

        if (count >= capacity) {
            capacity *= 2;
            tokens = (Token*)realloc(tokens, capacity * sizeof(Token));
        }

        Token* t = &tokens[count];
        t->type = parse_type_string(type_str);
        strncpy(t->text, text, 255);
        t->text[255] = '\0';
        t->line = line_num;
        t->length = strlen(text);

        count++;
    }

    fclose(f);

    // 添加 EOF Token
    if (count >= capacity) {
        capacity++;
        tokens = (Token*)realloc(tokens, capacity * sizeof(Token));
    }
    Token* eof = &tokens[count];
    eof->type = T_EOF;
    strcpy(eof->text, "EOF");
    eof->line = (count > 0) ? tokens[count - 1].line : 1;
    eof->length = 3;
    count++;

    *out_count = count;
    return tokens;
}