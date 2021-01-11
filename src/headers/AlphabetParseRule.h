#ifndef EKW_PARSER_ALPHABET_PARSE_RULE_H
#define EKW_PARSER_ALPHABET_PARSE_RULE_H

#include <stdio.h>
#include "ParseFramework.h"

ParseRule* AlphabetRule_Create(ParseScheme* scheme, char* str);

void AlphabetRule_Free(AlphabetParseRule* rule);

ParseResult AlphabetRule_Parse(AlphabetParseRule* rule, char* str, ParseResult* result_ret);

// void AlphabetRule_PrintDeep(AlphabetParseRule* rule, FILE* fout, size_t depth, size_t maxDepth, char* indentStr);
void AlphabetRule_Print(AlphabetParseRule* rule, FILE* fout);

#endif