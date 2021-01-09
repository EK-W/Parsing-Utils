#ifndef EKW_PARSER_STRING_PARSE_RULE_H
#define EKW_PARSER_STRING_PARSE_RULE_H

#include <stdio.h>
#include <stdarg.h>
#include "ParseFramework.h"

ParseRule* StringRule_Create(ParseScheme* scheme, char* str);

void StringRule_Free(StringParseRule* rule);

ParseResult StringRule_Parse(StringParseRule* rule, char* str, ParseResult* result_ret);

// void StringRule_PrintDeep(StringParseRule* rule, FILE* fout, size_t depth, size_t maxDepth, char* indentStr);
void StringRule_Print(StringParseRule* rule, FILE* fout);

#endif