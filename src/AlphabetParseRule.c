#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ParseFramework.h"


ParseRule* AlphabetRule_Create(ParseScheme* scheme, char* alphabet) {
	if((scheme == NULL) || (scheme->errorState != 0)) {
		return NULL;
	}

	ParseRule* ret = getSchemeSpaceForNewRule(scheme);

	if(ret == NULL) {
		return NULL;
	}

	ret->alphabetRule = (AlphabetParseRule*) malloc(sizeof(AlphabetParseRule));	

	if(ret->alphabetRule == NULL) {
		fprintf(stderr, "Error: unable to allocate alphabet rule!\n");
		ParseScheme_Free(scheme);
		scheme->errorState = 2;
		return NULL;
	}

	ret->ruleType = PARSE_RULE_ALPHABET;

	ret->alphabetRule->alphabet = alphabet;
	ret->alphabetRule->alphabetLen = strlen(alphabet);

	return ret;
}

void AlphabetRule_Free(AlphabetParseRule* rule) {
	if(rule == NULL) return;

	//free(rule->alphabetRule->alphabet);
	rule->alphabet = NULL;
	free(rule);
}

ParseResult AlphabetRule_Parse(AlphabetParseRule* rule, char* str, ParseResult* result_ret) {
	if(rule == NULL) {
		fprintf(stderr, "Error: attempting to parse using a null rule.\n");
		return setParseResult(result_ret, false, NULL, 0);
	}

	if(str[0] == '\0') {
		return setParseResult(result_ret, false, NULL, 0);
	}

	for(size_t i = 0; i < rule->alphabetLen; i++) {
		if(str[0] == rule->alphabet[i]) {
			return setParseResult(result_ret, true, str, 1);
		}
	}

	return setParseResult(result_ret, false, NULL, 0);
}

// void AlphabetRule_PrintDeep(AlphabetParseRule* rule, FILE* fout, size_t depth, size_t maxDepth, char* indentStr) {
// 	for(size_t i = 0; i < depth; i++) {
// 		fprintf(fout, "%s", indentStr);
// 	}

// 	fprintf(fout, "Alphabet(\"%s\")", rule->alphabet);
// }

void AlphabetRule_Print(AlphabetParseRule* rule, FILE* fout) {
	fprintf(fout, "Alphabet(\"%s\")", rule->alphabet);
}

