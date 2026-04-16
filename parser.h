// parser.h
#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

/**
 * 从 Token 数组解析出模块 AST。
 * @param tokens Token 数组（由 read_tokens_from_file 返回）
 * @param count  Token 数量
 * @return 成功返回 Module*，失败返回 NULL
 */
Module* parse(const Token* tokens, int count);

#endif