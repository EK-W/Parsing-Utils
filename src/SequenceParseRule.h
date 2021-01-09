#ifndef EKW_PARSER_SEQUENCE_PARSE_RULE_H
#define EKW_PARSER_SEQUENCE_PARSE_RULE_H

#include <stdio.h>
#include <stdarg.h>
#include "ParseFramework.h"

ParseRule* createSequenceRule(ParseScheme* scheme, ...);

#define SequenceRule_Create(scheme, ...) createSequenceRule(scheme, __VA_ARGS__, NULL)

void SequenceRule_Free(SequenceParseRule* rule);

ParseResult SequenceRule_Parse(SequenceParseRule* rule, char* str, ParseResult* result_ret);

void SequenceRule_PrintDeep(SequenceParseRule* rule, FILE* fout, size_t depth, size_t maxDepth, char* indentStr);
void SequenceRule_Print(SequenceParseRule* rule, FILE* fout);

#endif