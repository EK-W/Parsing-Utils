#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include "ParseFramework.h"
#include "AlphabetParseRule.h"
#include "OptionListParseRule.h"
#include "SequenceParseRule.h"
#include "StringParseRule.h"
#include "ForwardParseRule.h"
#include "OptionalParseRule.h"
#include "RepeatParseRule.h"

const size_t PARSE_SCHEME_BUFFER_LENGTH = 100;

ParseScheme* ParseScheme_Create() {
	ParseScheme* ret = (ParseScheme*) malloc(sizeof(ParseScheme));

	if(ret == NULL) {
		fprintf(stderr, "Error: unable to allocate parse scheme!\n");
		return NULL;
	}

	ret->rules = (ParseRule*) malloc(sizeof(ParseRule) * PARSE_SCHEME_BUFFER_LENGTH);

	if(ret == NULL) {
		fprintf(stderr, "Error: unable to allocate parse scheme!\n");
		free(ret);
		return NULL;
	}

	ret->numRules = 0;
	ret->maxRules = PARSE_SCHEME_BUFFER_LENGTH;
	ret->errorState = 0;
	ret->numUnresolvedForwardRules = 0;

	return ret;
}

void ParseScheme_Free(ParseScheme* scheme) {
	// Free individual rules.
	for(size_t i = 0; i < scheme->numRules; i++) {
		Rule_Free(&(scheme->rules[i]));
		//scheme->rules[i].ruleType = PARSE_RULE_NO_ALLOCATED_DATA;
	}

	free(scheme->rules);
	scheme->rules = NULL;
	scheme->errorState = -1;
}

ParseRule* getSchemeSpaceForNewRule(ParseScheme* scheme) {
	if((scheme == NULL) || (scheme->errorState != 0)) {
		return NULL;
	}


	if(scheme->numRules == scheme->maxRules) {
		size_t newLength = scheme->maxRules + PARSE_SCHEME_BUFFER_LENGTH;

		ParseRule* rulesPtr = (ParseRule*) realloc(scheme->rules, sizeof(ParseRule) * newLength);

		if(rulesPtr == NULL) {
			// Couldn't realloc. Let's try allocating new space, then.
			rulesPtr = (ParseRule*) malloc(sizeof(ParseRule) * newLength);

			if(rulesPtr == NULL) {
				fprintf(stderr, "Error: cannot allocate or reallocate required space in the parse scheme.\n");
				ParseScheme_Free(scheme);
				scheme->errorState = 1;
				return NULL;
			}

			// Ok. We have allocated new space. Let's copy the rules from the old space.
			memcpy(rulesPtr, scheme->rules, sizeof(ParseRule) * scheme->numRules);

			// Finally, let's free the old space now that we're no longer using it.
			free(scheme->rules);

			scheme->rules = rulesPtr;
		}
		scheme->maxRules = newLength;
	}

	// Ok. It is now guaranteed that there is space for the new rule.
	scheme->numRules++;

	// Initialize the space to default values
	scheme->rules[scheme->numRules - 1] = (ParseRule) {
		.ruleType = PARSE_RULE_NO_TYPE,
		.wasForwardDeclaration = false,
		.scheme = scheme
	};

	// return a pointer to the space
	return &(scheme->rules[scheme->numRules - 1]);
}

void ParseScheme_Print(ParseScheme* scheme, FILE* fout) {
	if(scheme == NULL) {
		fprintf(fout, "Scheme is null!\n");
		return;
	}
	if(scheme->errorState != 0) {
		fprintf(fout, "Scheme has error state %d!\n", scheme->errorState);
		return;
	}

	fprintf(fout, "Scheme has %lu/%lu rule slots filled.\n", scheme->numRules, scheme->maxRules);
	if(scheme->numUnresolvedForwardRules > 0) {
		fprintf(fout, "Scheme has %lu unresolved forward-declared rules.\n", scheme->numUnresolvedForwardRules);
	}

	for(size_t i = 0; i < scheme->numRules; i++)	{
		ParseRule* rule = scheme->rules + i;

		// fprintf(out, "| %p:\t", rule);

		Rule_Print(rule, fout);
		fprintf(fout, "\n");
	}
}



void Rule_Free(ParseRule* rule) {
	if(rule == NULL) {
		return;
	}

	if(rule->wasForwardDeclaration) {
		return;
	}

	switch(rule->ruleType) {
		case PARSE_RULE_ALPHABET:
			AlphabetRule_Free(rule->alphabetRule);
			rule->alphabetRule = NULL;
			break;
		case PARSE_RULE_OPTION_LIST:
			OptionListRule_Free(rule->optionListRule);
			rule->optionListRule = NULL;
			break;
		case PARSE_RULE_SEQUENCE:
			SequenceRule_Free(rule->sequenceRule);
			rule->sequenceRule = NULL;
			break;
		case PARSE_RULE_STRING:
			StringRule_Free(rule->stringRule);
			rule->stringRule = NULL;
			break;
		case PARSE_RULE_OPTIONAL:
			OptionalRule_Free(rule->optionalRule);
			rule->optionalRule = NULL;
			break;
		case PARSE_RULE_REPEAT:
			RepeatRule_Free(rule->repeatRule);
			rule->repeatRule = NULL;
			break;
		default:
			fprintf(stderr, "Error: I don't know how to free that type of parse rule.\n");
			break;
	}
}

ParseResult Rule_Parse(ParseRule* rule, char* str, ParseResult* result_ret) {
	if(rule == NULL) {
		fprintf(stderr, "Error: attempting to parse using a null rule.\n");
		return setParseResult(result_ret, false, NULL, 0);
	}

	switch(rule->ruleType) {
		case PARSE_RULE_ALPHABET:
			return AlphabetRule_Parse(rule->alphabetRule, str, result_ret);
		case PARSE_RULE_OPTION_LIST:
			return OptionListRule_Parse(rule->optionListRule, str, result_ret);
		case PARSE_RULE_SEQUENCE:
			return SequenceRule_Parse(rule->sequenceRule, str, result_ret);
		case PARSE_RULE_STRING:
			return StringRule_Parse(rule->stringRule, str, result_ret);
		case PARSE_RULE_FORWARD_DECLARED:
			fprintf(stderr,
				"Error: attempting to parse using a forward declared rule that hasn't been given a value! "
				"Make sure to eventually call Rule_SetForwardRuleValue for every forward rule that you declare.\n"
			);
			return setParseResult(result_ret, false, NULL, 0);
		case PARSE_RULE_OPTIONAL:
			return OptionalRule_Parse(rule->optionalRule, str, result_ret);
		case PARSE_RULE_REPEAT:
			return RepeatRule_Parse(rule->repeatRule, str, result_ret);
		default:
			fprintf(stderr, "Error: I don't know how to parse using that rule.\n");
			return setParseResult(result_ret, false, NULL, 0);
	}
}

void Rule_PrintSimpleRulePointer(ParseRule* rule, FILE* fout) {
	if(rule == NULL) {
		fprintf(fout, "NULL");
		return;
	}

	ptrdiff_t ruleOffset = rule - rule->scheme->rules;
	size_t maxRuleOffset = rule->scheme->numRules;

	int maxRuleOffsetPrintLength = (maxRuleOffset == 0)? 1 : ((int) (log(maxRuleOffset) / log(16))) + 1;

	fprintf(fout, "0x%0*lX", maxRuleOffsetPrintLength, ruleOffset);
}

void Rule_PrintDeep(ParseRule* rule, FILE* fout, size_t depth, size_t maxDepth, char* indentStr) {
	for(size_t i = 0; i < depth; i++) {
		fprintf(fout, "%s", indentStr);
	}

	//fprintf(fout, "%p: ", rule);
	Rule_PrintSimpleRulePointer(rule, fout);
	fprintf(fout, ": ");

	if(rule->wasForwardDeclaration) {
		fprintf(fout, "(forward) ");
	}

	switch(rule->ruleType) {
		case PARSE_RULE_ALPHABET:
			AlphabetRule_Print(rule->alphabetRule, fout);
			break;
		case PARSE_RULE_OPTION_LIST:
			OptionListRule_PrintDeep(rule->optionListRule, fout, depth, maxDepth, indentStr);
			break;
		case PARSE_RULE_SEQUENCE:
			SequenceRule_PrintDeep(rule->sequenceRule, fout, depth, maxDepth, indentStr);
			break;
		case PARSE_RULE_FORWARD_DECLARED:
			ForwardRule_Print(rule, fout);
			break;
		case PARSE_RULE_STRING:
			StringRule_Print(rule->stringRule, fout);
			break;
		case PARSE_RULE_OPTIONAL:
			OptionalRule_PrintDeep(rule->optionalRule, fout, depth, maxDepth, indentStr);
			break;
		case PARSE_RULE_REPEAT:
			RepeatRule_PrintDeep(rule->repeatRule, fout, depth, maxDepth, indentStr);
			break;
		default:
			fprintf(fout, "Unknown Rule Type\n");
			break;
	}
}

void Rule_Print(ParseRule* rule, FILE* fout) {
	Rule_PrintDeep(rule, fout, 0, 0, "");
}

ParseResult setParseResult(ParseResult* ptr, bool success, char* str, size_t length) {
	ParseResult result = {
		.success = success,
		.str = str,
		.length = length
	};
	
	if(ptr != NULL) {
		(*ptr) = result;
	}

	return result;
}