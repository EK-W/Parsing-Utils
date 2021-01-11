#ifndef EKW_PARSER_OPTIONAL_PARSE_RULE_H
#define EKW_PARSER_OPTIONAL_PARSE_RULE_H

#include <stdio.h>
#include "ParseFramework.h"

ParseRule* OptionalRule_Create(ParseScheme* scheme, ParseRule* rule);

void OptionalRule_Free(OptionalParseRule* rule);

ParseResult OptionalRule_Parse(OptionalParseRule* rule, char* str, ParseResult* result_ret);

void OptionalRule_PrintDeep(OptionalParseRule* rule, FILE* fout, size_t depth, size_t maxDepth, char* indentStr);
void OptionalRule_Print(OptionalParseRule* rule, FILE* fout);

#endif