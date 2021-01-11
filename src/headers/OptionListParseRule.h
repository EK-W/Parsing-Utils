#ifndef EKW_PARSER_OPTION_LIST_PARSE_RULE_H
#define EKW_PARSER_OPTION_LIST_PARSE_RULE_H

#include <stdio.h>
#include <stdarg.h>
#include "ParseFramework.h"

ParseRule* createOptionListRule(ParseScheme* scheme, ...);

#define OptionListRule_Create(scheme, ...) createOptionListRule(scheme, __VA_ARGS__, NULL)

void OptionListRule_Free(OptionListParseRule* rule);

ParseResult OptionListRule_Parse(OptionListParseRule* rule, char* str, ParseResult* result_ret);

void OptionListRule_PrintDeep(OptionListParseRule* rule, FILE* fout, size_t depth, size_t maxDepth, char* indentStr);
void OptionListRule_Print(OptionListParseRule* rule, FILE* fout);

#endif