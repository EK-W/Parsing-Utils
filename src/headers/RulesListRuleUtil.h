#ifndef EKW_PARSER_RULES_LIST_RULE_UTIL_H
#define EKW_PARSER_RULES_LIST_RULE_UTIL_H

#include <stdio.h>
#include "ParseFramework.h"


ParseRule* RulesListRuleData_Create(ParseScheme* scheme, va_list varArgs, ParseRuleType ruleType);

void RulesListRuleData_Free(RulesListRuleData* data);

void RulesListRuleData_PrintDeep(RulesListRuleData* data, FILE* fout, size_t depth, size_t maxDepth, char* indentStr);

#endif