#include <stdio.h>
#include <stdlib.h>
#include "ParseFramework.h"


ParseRule* RepeatRule_CreateWithBounds(ParseScheme* scheme, size_t minReps, size_t maxReps, ParseRule* rule) {
	if((scheme == NULL) || (scheme->errorState != 0)) {
		return NULL;
	}

	ParseRule* ret = getSchemeSpaceForNewRule(scheme);

	if(ret == NULL) {
		return NULL;
	}

	ret->repeatRule = (RepeatParseRule*) malloc(sizeof(RepeatParseRule));

	if(ret->repeatRule == NULL) {
		fprintf(stderr, "Error: unable to allocate repeat rule!\n");
		ParseScheme_Free(scheme);
		scheme->errorState = 2;
		return NULL;
	}

	ret->repeatRule->rule = rule;
	ret->repeatRule->minReps = minReps;
	ret->repeatRule->maxReps = maxReps;

	ret->ruleType = PARSE_RULE_REPEAT;

	return ret;
}

ParseRule* RepeatRule_Create(ParseScheme* scheme, bool required, ParseRule* rule) {
	return RepeatRule_CreateWithBounds(scheme, required? 1 : 0, SIZE_MAX, rule);
}

void RepeatRule_Free(RepeatParseRule* rule) {
	free(rule);
}

ParseResult RepeatRule_Parse(RepeatParseRule* rule, char* str, ParseResult* result_ret) {
	if(rule == NULL) {
		fprintf(stderr, "Error: attempting to parse using a null rule.\n");
		return setParseResult(result_ret, false, NULL, 0);
	}

	size_t strIndex = 0;
	size_t numReps = 0;

	for(; numReps <= rule->maxReps; numReps++) {
		ParseResult res;
		if(Rule_Parse(rule->rule, str + strIndex, &res).success) {
			strIndex += res.length;
		} else {
			break;
		}
	}

	if(numReps < rule->minReps) {
		return setParseResult(result_ret, false, NULL, 0);
	}

	return setParseResult(result_ret, true, str, strIndex);
}

void RepeatRule_PrintDeep(RepeatParseRule* rule, FILE* fout, size_t depth, size_t maxDepth, char* indentStr) {
	fprintf(fout, "Repeat(");
	Rule_PrintSimpleRulePointer(rule->rule, fout);
	fprintf(fout, ")");

	if(depth < maxDepth) {
		Rule_PrintDeep(rule->rule, fout, depth + 1, maxDepth, indentStr);
	}
}

void RepeatRule_Print(RepeatParseRule* rule, FILE* fout) {
	RepeatRule_PrintDeep(rule, fout, 0, 0, "");
}