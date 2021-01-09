#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "ParseFramework.h"

const size_t PARSE_RULE_LIST_BUFFER_LENGTH = 100;

ParseRule* RulesListRuleData_Create(ParseScheme* scheme, va_list varArgs, ParseRuleType ruleType) {
	if((scheme == NULL) || (scheme->errorState != 0)) {
		return NULL;
	}
	if((ruleType != PARSE_RULE_OPTION_LIST) && (ruleType != PARSE_RULE_SEQUENCE)) {
		fprintf(stderr, "Error: passing an invalid ruleType to buildRulesListRuleData.\n");

		ParseScheme_Free(scheme);
		return NULL;
	}

	ParseRule* ret = getSchemeSpaceForNewRule(scheme);

	if(ret == NULL) {
		return NULL;
	} 

	ParseRule* ruleBuffer[PARSE_RULE_LIST_BUFFER_LENGTH];
	size_t numRules = 0;

	ParseRule* nextRule = va_arg(varArgs, ParseRule*);

	do {
		while((nextRule != NULL) && (numRules < PARSE_RULE_LIST_BUFFER_LENGTH)) {
			ruleBuffer[numRules] = nextRule;
			numRules++;

			nextRule = va_arg(varArgs, ParseRule*);
		}

		
		RulesListRuleData* ruleData = (RulesListRuleData*) malloc(sizeof(RulesListRuleData));
		ParseRule** rulesList = (ParseRule**) malloc(sizeof(ParseRule*) * numRules);

		if((ruleData == NULL) || (rulesList == NULL)) {
			fprintf(stderr, "Error: unable to allocate Rule List rule!\n");

			free(rulesList);
			free(ruleData);
			ParseScheme_Free(scheme);
			scheme->errorState = 2;	

			return NULL;
		}

		memcpy(rulesList, ruleBuffer, sizeof(ParseRule*) * numRules);

		ruleData->rules = rulesList;
		ruleData->rulesLen = numRules;
		if(ruleType == PARSE_RULE_OPTION_LIST) {
			ret->optionListRule = ruleData;
		} else if(ruleType == PARSE_RULE_SEQUENCE){
			ret->sequenceRule = ruleData;
		}
		ret->ruleType = ruleType;

		// If there are still more rules to parse, make this rule list rule be a child of a new rule list rule
		// and continue parsing the arguments.
		if(nextRule != NULL) {
			numRules = 1;
			ruleBuffer[0] = ret;

			ret = getSchemeSpaceForNewRule(scheme);

			if(ret == NULL) {
				return NULL;
			}
		}
	} while(nextRule != NULL);

	// Ok. Once we get to this point, we have run out of varargs. Our rule list rule is complete.

	return ret;
}

void RulesListRuleData_Free(RulesListRuleData* data) {
	if(data == NULL) return;
			
	free(data->rules);
	data->rules = NULL;
	free(data);
}

void RulesListRuleData_PrintDeep(RulesListRuleData* data, FILE* fout, size_t depth, size_t maxDepth, char* indentStr) {
	fprintf(fout, "(");
	if(data->rulesLen > 0) {
		fprintf(fout, "%p", data->rules[0]);
	}
	for(size_t i = 1; i < data->rulesLen; i++) {
		fprintf(fout, ", %p", data->rules[i]);
	}
	fprintf(fout, ")");

	if(depth < maxDepth) {
		for(size_t i = 0; i < data->rulesLen; i++) {
			fprintf(fout, "\n");
			Rule_PrintDeep(data->rules[i], fout, depth + 1, maxDepth, indentStr);
		}
	}
}