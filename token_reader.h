// token_reader.h
#ifndef TOKEN_READER_H
#define TOKEN_READER_H

#include "lexer.h"

/**
 * 从 token_output.txt 文件中读取 Token 序列。
 * @param filename token 文件名（通常为 "token_output.txt"）
 * @param out_count 输出参数，返回 Token 数量
 * @return 动态分配的 Token 数组，需调用 free 释放
 */
Token* read_tokens_from_file(const char* filename, int* out_count);

#endif