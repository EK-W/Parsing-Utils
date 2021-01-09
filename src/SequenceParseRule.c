#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ParseFramework.h"
#include "RulesListRuleUtil.h"

ParseRule* createSequenceRule(ParseScheme* scheme, ...) {
	if((scheme == NULL) || (scheme->errorState != 0)) {
		return NULL;
	}

	va_list varArgs;
	va_start(varArgs, scheme);

	ParseRule* ret = RulesListRuleData_Create(scheme, varArgs, PARSE_RULE_SEQUENCE);

	va_end(varArgs);

	return ret;
}

void SequenceRule_Free(SequenceParseRule* rule) {
	RulesListRuleData_Free(rule);
}

ParseResult SequenceRule_Parse(SequenceParseRule* rule, char* str, ParseResult* result_ret) {
	if(rule == NULL) {
		fprintf(stderr, "Error: attempting to parse using a null rule.\n");
		return setParseResult(result_ret, false, NULL, 0);
	}

	size_t strIndex = 0;

	for(size_t i = 0; i < rule->rulesLen; i++) {
		ParseResult result;
		if(!(Rule_Parse(rule->rules[i], str + strIndex, &result).success)) {
			return setParseResult(result_ret, false, NULL, 0);
		}

		strIndex += result.length;
	}

	return setParseResult(result_ret, true, str, strIndex);
}

void SequenceRule_PrintDeep(SequenceParseRule* rule, FILE* fout, size_t depth, size_t maxDepth, char* indentStr) {
	// for(size_t i = 0; i < depth; i++) {
	// 	fprintf(fout, "%s", indentStr);
	// }

	fprintf(fout, "Sequence");
	RulesListRuleData_PrintDeep(rule, fout, depth, maxDepth, indentStr);
}

void SequenceRule_Print(SequenceParseRule* rule, FILE* fout) {
	SequenceRule_PrintDeep(rule, fout, 0, 0, "");
}

