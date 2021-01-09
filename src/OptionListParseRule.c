#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ParseFramework.h"
#include "RulesListRuleUtil.h"


ParseRule* createOptionListRule(ParseScheme* scheme, ...) {
	if((scheme == NULL) || (scheme->errorState != 0)) {
		return NULL;
	}

	va_list varArgs;
	va_start(varArgs, scheme);

	ParseRule* ret = RulesListRuleData_Create(scheme, varArgs, PARSE_RULE_OPTION_LIST);

	va_end(varArgs);

	return ret;
}

void OptionListRule_Free(OptionListParseRule* rule) {
	RulesListRuleData_Free(rule);
}

ParseResult OptionListRule_Parse(OptionListParseRule* rule, char* str, ParseResult* result_ret) {
	if(rule == NULL) {
		fprintf(stderr, "Error: attempting to parse using a null rule.\n");
		return setParseResult(result_ret, false, NULL, 0);
	}

	for(size_t i = 0; i < rule->rulesLen; i++) {
		ParseResult result; 
		if(Rule_Parse(rule->rules[i], str, &result).success) {
			(*result_ret) = result;
			return result;
		}
	}

	return setParseResult(result_ret, false, NULL, 0);
}

void OptionListRule_PrintDeep(OptionListParseRule* rule, FILE* fout, size_t depth, size_t maxDepth, char* indentStr) {
	// for(size_t i = 0; i < depth; i++) {
	// 	fprintf(fout, "%s", indentStr);
	// }

	fprintf(fout, "Options");
	RulesListRuleData_PrintDeep(rule, fout, depth, maxDepth, indentStr);
}

void OptionListRule_Print(OptionListParseRule* rule, FILE* fout) {
	OptionListRule_PrintDeep(rule, fout, 0, 0, "");
}