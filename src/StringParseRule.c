#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ParseFramework.h"

ParseRule* StringRule_Create(ParseScheme* scheme, char* str) {
	if((scheme == NULL) || (scheme->errorState != 0)) {
		return NULL;
	}

	ParseRule* ret = getSchemeSpaceForNewRule(scheme);

	if(ret == NULL) {
		return NULL;
	}

	size_t stringLen = strlen(str);

	StringParseRule* ruleData = (StringParseRule*) malloc(sizeof(StringParseRule));
	char* strCopy = (char*) malloc(sizeof(char) * stringLen);

	if((ruleData == NULL) || (strCopy == NULL)) {
		fprintf(stderr, "Error: unable to allocate string rule!\n");

		free(ruleData);
		free(strCopy);

		ParseScheme_Free(scheme);
		scheme->errorState = 2;

		return NULL;
	}

	memcpy(strCopy, str, sizeof(char) * stringLen);

	ruleData->stringLen = stringLen;
	ruleData->string = strCopy;

	ret->stringRule = ruleData;
	ret->ruleType = PARSE_RULE_STRING;

	return ret;
}

void StringRule_Free(StringParseRule* rule) {
	if(rule == NULL) return;
	
	free(rule->string);
	rule->string = NULL;
	free(rule);
}

ParseResult StringRule_Parse(StringParseRule* rule, char* str, ParseResult* result_ret) {
	if(rule == NULL) {
		fprintf(stderr, "Error: attempting to parse using a null rule.\n");
		return setParseResult(result_ret, false, NULL, 0);
	}

	if(strncmp(str, rule->string, rule->stringLen) == 0) {
		return setParseResult(result_ret, true, str, rule->stringLen);
	} else {
		return setParseResult(result_ret, false, NULL, 0);
	}
}


// void StringRule_PrintDeep(StringParseRule* rule, FILE* fout, size_t depth, size_t maxDepth, char* indentStr) {
// 	for(size_t i = 0; i < depth; i++) {
// 		fprintf(fout, "%s", indentStr);
// 	}

// 	fprintf(fout, "String(\"%s\")", rule->string);
// }

void StringRule_Print(StringParseRule* rule, FILE* fout) {
	fprintf(fout, "String(\"%s\")", rule->string);
}
