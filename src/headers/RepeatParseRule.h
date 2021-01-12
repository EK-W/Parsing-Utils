#ifndef EKW_PARSER_REPEAT_PARSE_RULE_H
#define EKW_PARSER_REPEAT_PARSE_RULE_H

#include <stdio.h>
#include <stdbool.h>
#include "ParseFramework.h"

ParseRule* RepeatRule_CreateWithBounds(ParseScheme* scheme, size_t minReps, size_t maxReps, ParseRule* rule);
ParseRule* RepeatRule_Create(ParseScheme* scheme, bool required, ParseRule* rule);

void RepeatRule_Free(RepeatParseRule* rule);

ParseResult RepeatRule_Parse(RepeatParseRule* rule, char* str, ParseResult* result_ret);

void RepeatRule_Print(RepeatParseRule* rule, FILE* fout);

void RepeatRule_PrintDeep(RepeatParseRule* rule, FILE* fout, size_t depth, size_t maxDepth, char* indentStr);

#endif