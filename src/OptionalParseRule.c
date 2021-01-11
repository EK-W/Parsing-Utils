#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "ParseFramework.h"

ParseRule* OptionalRule_Create(ParseScheme* scheme, ParseRule* rule) {
	if((scheme == NULL) || (scheme->errorState != 0)) {
		return NULL;
	}

	ParseRule* ret = getSchemeSpaceForNewRule(scheme);

	if(ret == NULL) {
		return NULL;
	}

	ret->optionalRule = (OptionalParseRule*) malloc(sizeof(OptionalParseRule));

	if(ret->optionalRule == NULL) {
		fprintf(stderr, "Error: unable to allocate optional rule!\n");
		ParseScheme_Free(scheme);
		scheme->errorState = 2;
		return NULL;
	}

	ret->optionalRule->rule = rule;

	ret->ruleType = PARSE_RULE_OPTIONAL;

	return ret;
}

void OptionalRule_Free(OptionalParseRule* rule) {
	if(rule == NULL) {
		return;
	}

	free(rule);
}

ParseResult OptionalRule_Parse(OptionalParseRule* rule, char* str, ParseResult* result_ret) {
	ParseResult parseRes;
	if(Rule_Parse(rule->rule, str, &parseRes).success) {
		if(result_ret != NULL) {
			(*result_ret) = parseRes;
		}
		return parseRes;
	} else {
		return setParseResult(result_ret, true, str, 0);
	}
}

void OptionalRule_PrintDeep(OptionalParseRule* rule, FILE* fout, size_t depth, size_t maxDepth, char* indentStr) {
	fprintf(fout, "Optional(");
	Rule_PrintSimpleRulePointer(rule->rule, fout);
	fprintf(fout, ")");

	if(depth < maxDepth) {
		Rule_PrintDeep(rule->rule, fout, depth + 1, maxDepth, indentStr);
	}

}
void OptionalRule_Print(OptionalParseRule* rule, FILE* fout) {
	OptionalRule_PrintDeep(rule, fout, 0, 0, "");
}