#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#define Rule_OptionList(scheme, ...) createOptionList(scheme, __VA_ARGS__, NULL)

const size_t PARSE_RULE_LIST_BUFFER_LENGTH = 100;
const size_t PARSE_SCHEME_BUFFER_LENGTH = 100;

/*
Every time you create a new Parse Rule type:
1) Write a function to create it.
2) Add it to the freeParseRule function.
3) Add it to the Rule_Print function.
4) Add it to the parseWithRule function
*/

typedef enum {
	PARSE_RULE_ALPHABET,
	PARSE_RULE_OPTION_LIST,
	PARSE_RULE_SEQUENCE,
	PARSE_RULE_FORWARD_DECLARED,
	PARSE_RULE_STRING
	// PARSE_RULE_REPEAT
	// PARSE_RULE_OPTIONAL

	// // The uninitialized rule type is allowed ONLY for rules whose data has either been
	// // deallocated or never allocated in the first place.
	// PARSE_RULE_NO_ALLOCATED_DATA
} ParseRuleType;

typedef struct ParseRule_s ParseRule;

typedef struct {
	const char* alphabet;
	size_t alphabetLen;
} AlphabetParseRule;

typedef struct {
	ParseRule** rules;
	size_t rulesLen;
} RulesListRuleData;

typedef RulesListRuleData OptionListParseRule;
typedef RulesListRuleData SequenceParseRule;

// typedef struct {
// 	ParseRule** options;
// 	size_t optionsLen;
// } OptionListParseRule;

// typedef struct {
// 	ParseRule** sequence;
// 	size_t sequenceLen;
// } SequenceParseRule;

typedef struct {
	char* string;
	size_t stringLen;
} StringParseRule;

// typedef struct {
// 	ParseRule* toRepeat;
// 	size_t maxRepetitions;
// 	size_t minRepetitions;
// } RepeatParseRule;


struct ParseRule_s{
	// It's generally a good idea to only set a new rule's type *after* you have allocated its data.
	// This will help avoid bugs where we try to free data that hasn't been allocated
	ParseRuleType ruleType;

	bool wasForwardDeclaration;

	union {
		AlphabetParseRule* alphabetRule;
		OptionListParseRule* optionListRule;
		SequenceParseRule* sequenceRule;
		StringParseRule* stringRule;
		// RepeatParseRule* repeatRule;
	};
};

typedef struct {
	ParseRule* rules;
	size_t numRules;
	size_t maxRules;


	/* Here are the meanings of the errorState values:
	-1	| The ParseScheme has been freed.
	 0	| The ParseScheme has not been freed or encountered any errors.
	 1	| The ParseScheme could not allocate more space for a rule.
	 2	| There was an error while allocating space for a rule's data.
	 3	| An argument to a function was null when it should not have been.
	 4	| An argument to a function was in some way not a valid argument for that function.
	*/
	int errorState;

	size_t numUnresolvedForwardRules;
} ParseScheme;

/*
Here's how recursion will work:
	There will be a function called Rule_ForwardDeclare. It will return a ParseRule* of type ForwardDeclared.
		It will not allocate this rule from the normal pool. ParseScheme will have a separate pool for forward
		declared rules. Or maybe not. Doesn't really matter.
	There will be a function called Rule_SetForwardRuleValue. It will take in the scheme, a forward declared ruleptr,
		and another ruleptr ("rule value"). It will copy the ruleValue into the forward declared rule. This will
		resolve the forward rule!
*/

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

void freeParseRule(ParseRule* rule) {
	if(rule == NULL) {
		return;
	}

	if(rule->wasForwardDeclaration) {
		return;
	}

	switch(rule->ruleType) {
		case PARSE_RULE_ALPHABET:
			if(rule->alphabetRule == NULL) return;

			//free(rule->alphabetRule->alphabet);
			rule->alphabetRule->alphabet = NULL;
			free(rule->alphabetRule);
			rule->alphabetRule = NULL;
			break;
		case PARSE_RULE_OPTION_LIST:
			if(rule->optionListRule == NULL) return;

			free(rule->optionListRule->rules);
			rule->optionListRule->rules = NULL;

			free(rule->optionListRule);
			rule->optionListRule = NULL;
			break;
		case PARSE_RULE_SEQUENCE:
			if(rule->sequenceRule == NULL) return;
			
			free(rule->sequenceRule->rules);
			rule->sequenceRule->rules = NULL;
			free(rule->sequenceRule);
			rule->sequenceRule = NULL;
			break;
		case PARSE_RULE_STRING:
			if(rule->stringRule == NULL) return;
			
			free(rule->stringRule->string);
			rule->stringRule->string = NULL;
			free(rule->stringRule);
			rule->stringRule = NULL;
			break;
		default:
			fprintf(stderr, "Error: I don't know how to free that type of parse rule.\n");
			break;
	}
}

// NOTE! To free a scheme, it is necessary to call this function AND to call free(scheme).
void ParseScheme_Free(ParseScheme* scheme) {
	// Free individual rules.
	for(size_t i = 0; i < scheme->numRules; i++) {
		freeParseRule(&(scheme->rules[i]));
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

	// Null initialize the space
	//scheme->rules[scheme->numRules - 1].type = PARSE_RULE_NO_ALLOCATED_DATA;
	scheme->rules[scheme->numRules - 1].wasForwardDeclaration = false;

	// return a pointer to the space
	return &(scheme->rules[scheme->numRules - 1]);
}

// This function can be called at any point (provided it isn't somehow called in the middle of a rule definition).
// It will resolve any recursive calls in the scheme. It is required to call this function before you try to parse using
// a recursive rule. Technically, it is fastest to call this function immediately after defining a recursive
// rule. But it is fine if you don't call it immediately. Just make sure you call it before you try to parse, though.
// void ParseScheme_ResolveRecursion(ParseScheme* scheme) {
// 	if((scheme == NULL) || (scheme->errorState != 0)) {
// 		return;
// 	}

// 	// If there are no unresolved recursive rules, return.
// 	if(scheme->firstRecursiveRuleIndex == SIZE_MAX) {
// 		return;
// 	}

// 	size_t index = scheme->firstRecursiveRuleIndex;

// 	while(index < scheme->numRules) {
// 		ParseRule* recursionRule = &(scheme->rules[index]);

// 		if(recursionRule->ruleType != PARSE_RULE_RECURSE) {
// 			continue;
// 		}

// 		// TODO: Might be good to do some error/null checking

// 		ParseRule* ruleToRecurseTo = *(recursionRule->recursiveRule);

// 		(*recursionRule) = (*ruleToRecurseTo);

// 		index++;
// 	}

// 	scheme->firstRecursiveRuleIndex = SIZE_MAX;
// }

ParseRule* Rule_Alphabet(ParseScheme* scheme, const char* alphabet) {
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

ParseRule* buildRulesListRuleData(ParseScheme* scheme, va_list varArgs, ParseRuleType ruleType) {
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

ParseRule* createOptionList(ParseScheme* scheme, ...) {
	if((scheme == NULL) || (scheme->errorState != 0)) {
		return NULL;
	}

	va_list varArgs;
	va_start(varArgs, scheme);

	ParseRule* ret = buildRulesListRuleData(scheme, varArgs, PARSE_RULE_OPTION_LIST);

	va_end(varArgs);

	return ret;
}

ParseRule* Rule_Sequence(ParseScheme* scheme, ...) {
	if((scheme == NULL) || (scheme->errorState != 0)) {
		return NULL;
	}

	va_list varArgs;
	va_start(varArgs, scheme);

	ParseRule* ret = buildRulesListRuleData(scheme, varArgs, PARSE_RULE_SEQUENCE);

	va_end(varArgs);

	return ret;
}

// ParseRule* Rule_BoundedRepeat(ParseScheme* scheme, size_t min, size_t max, ParseRule* rule) {
// 	if((scheme == NULL) || (scheme->errorState != 0)) {
// 		return NULL;
// 	}

// 	if(rule == NULL) {
// 		fprintf(stderr, "Error: rule to repeat cannot be NULL!\n");

// 		ParseScheme_Free(scheme);
// 		scheme->errorState = 3;

// 		return NULL;
// 	}

// 	ParseRule* ret = getSchemeSpaceForNewRule(scheme);

// 	if(ret == NULL) {
// 		return NULL;
// 	}

// 	RepeatParseRule* ruleData = (RepeatParseRule*) malloc(sizeof(RepeatParseRule));

// 	if(ruleData == NULL) {
// 		fprintf(stderr, "Error: unable to allocate repeat rule!\n");

// 		ParseScheme_Free(scheme);
// 		scheme->errorState = 2;

// 		return NULL;
// 	}

// 	ret->repeatRule = ruleData;
// 	ret->ruleType = PARSE_RULE_REPEAT;
// 	ruleData->toRepeat = rule;
// 	ruleData->minRepetitions = min;
// 	ruleData->maxRepetitions = max;

// 	return ret;
// }

// ParseRule* Rule_Repeat(ParseScheme* scheme, ParseRule* rule) {
// 	return Rule_BoundedRepeat(scheme, 1, SIZE_MAX, rule);
// }

ParseRule* Rule_ForwardDeclare(ParseScheme* scheme) {
	if((scheme == NULL) || (scheme->errorState != 0)) {
		return NULL;
	}

	ParseRule* ret = getSchemeSpaceForNewRule(scheme);

	if(ret == NULL) {
		return NULL;
	}

	ret->ruleType = PARSE_RULE_FORWARD_DECLARED;
	ret->wasForwardDeclaration = true;

	scheme->numUnresolvedForwardRules++;

	return ret;
}

ParseRule* Rule_SetForwardRuleValue(ParseScheme* scheme, ParseRule* forwardRule, ParseRule* ruleValue) {
	if((scheme == NULL) || (scheme->errorState != 0)) {
		return NULL;
	}

	if(forwardRule == NULL) {
		fprintf(stderr, "Error: Attempting to set the value of a null forward rule!\n");

		ParseScheme_Free(scheme);
		scheme->errorState = 3;

		return NULL;
	}

	if(forwardRule->ruleType != PARSE_RULE_FORWARD_DECLARED) {
		fprintf(stderr,
			"Error: Attempting to use Rule_SetForwardRuleValue on a non-forward rule! "
			"(note: If you have already called Rule_SetForwardRuleValue on this rule, it is no longer "
			"considered to be a forward rule.)\n"
		);

		ParseScheme_Free(scheme);
		scheme->errorState = 4;

		return NULL;
	}

	if(ruleValue == NULL) {
		fprintf(stderr, "Error: Attempting to set a null value to a forward rule!\n");

		ParseScheme_Free(scheme);
		scheme->errorState = 3;

		return NULL;
	}

	(*forwardRule) = (*ruleValue);
	forwardRule->wasForwardDeclaration = true;

	scheme->numUnresolvedForwardRules--;

	return ruleValue;
}

ParseRule* Rule_String(ParseScheme* scheme, char* str) {
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

// ParseRule* Rule_Recurse(ParseScheme* scheme, ParseRule** rule) {
// 	if((scheme == NULL) || (scheme->errorState != 0)) {
// 		return NULL;
// 	}

// 	ParseRule* ret = getSchemeSpaceForNewRule(scheme);

// 	if(ret == NULL) {
// 		return NULL;
// 	}

// 	ret->recurseRule = rule;
// 	ret->ruleType = PARSE_RULE_RECURSE;

// 	return ret;
// }


typedef struct {
	bool success;
	char* str;
	size_t length;
} ParseResult;

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

ParseResult parseWithRule(char* str, ParseResult* result_ret, ParseRule* rule);

ParseResult parseWithAlphabetRule(char* str, ParseResult* result_ret, AlphabetParseRule* rule) {
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

ParseResult parseWithOptionListRule(char* str, ParseResult* result_ret, OptionListParseRule* rule) {

	if(rule == NULL) {
		fprintf(stderr, "Error: attempting to parse using a null rule.\n");
		return setParseResult(result_ret, false, NULL, 0);
	}

	for(size_t i = 0; i < rule->rulesLen; i++) {
		ParseResult result; 
		if(parseWithRule(str, &result, rule->rules[i]).success) {
			(*result_ret) = result;
			return result;
		}
	}

	return setParseResult(result_ret, false, NULL, 0);
}

ParseResult parseWithSequenceRule(char* str, ParseResult* result_ret, SequenceParseRule* rule) {
	if(rule == NULL) {
		fprintf(stderr, "Error: attempting to parse using a null rule.\n");
		return setParseResult(result_ret, false, NULL, 0);
	}

	size_t strIndex = 0;

	for(size_t i = 0; i < rule->rulesLen; i++) {
		ParseResult result;
		if(!(parseWithRule(str + strIndex, &result, rule->rules[i]).success)) {
			return setParseResult(result_ret, false, NULL, 0);
		}

		strIndex += result.length;
	}

	return setParseResult(result_ret, true, str, strIndex);
}

ParseResult parseWithStringRule(char* str, ParseResult* result_ret, StringParseRule* rule) {
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

ParseResult parseWithRule(char* str, ParseResult* result_ret, ParseRule* rule) {
	if(rule == NULL) {
		fprintf(stderr, "Error: attempting to parse using a null rule.\n");
		return setParseResult(result_ret, false, NULL, 0);
	}

	switch(rule->ruleType) {
		case PARSE_RULE_ALPHABET:
			return parseWithAlphabetRule(str, result_ret, rule->alphabetRule);
		case PARSE_RULE_OPTION_LIST:
			return parseWithOptionListRule(str, result_ret, rule->optionListRule);
		case PARSE_RULE_SEQUENCE:
			return parseWithSequenceRule(str, result_ret, rule->sequenceRule);
		case PARSE_RULE_STRING:
			return parseWithStringRule(str, result_ret, rule->stringRule);
		case PARSE_RULE_FORWARD_DECLARED:
			fprintf(stderr,
				"Error: attempting to parse using a forward declared rule that hasn't been given a value! "
				"Make sure to eventually call Rule_SetForwardRuleValue for every forward rule that you declare.\n"
			);
			return setParseResult(result_ret, false, NULL, 0);
		default:
			fprintf(stderr, "Error: I don't know how to parse using that rule.\n");
			return setParseResult(result_ret, false, NULL, 0);
	}
}

void Rule_Print(FILE* out, ParseRule* rule, size_t maxDepth, size_t depth);

void printRulesListRule(FILE* out, RulesListRuleData* ruleData, size_t maxDepth, size_t depth) {
	fprintf(out, "(");
	if(ruleData->rulesLen > 0) {
		fprintf(out, "%p", ruleData->rules[0]);
	}
	for(size_t i = 1; i < ruleData->rulesLen; i++) {
		fprintf(out, ", %p", ruleData->rules[i]);
	}
	fprintf(out, ")");

	if(depth < maxDepth) {
		for(size_t i = 0; i < ruleData->rulesLen; i++) {
			fprintf(out, "\n");
			Rule_Print(out, ruleData->rules[i], maxDepth, depth + 1);
		}
	}
}

void Rule_Print(FILE* out, ParseRule* rule, size_t maxDepth, size_t depth) {
	for(size_t i = 0; i < depth; i++) {
		fprintf(out, "|\t");
	}

	fprintf(out, "%p: ", rule);

	if(rule == NULL) {
		fprintf(out, "NULL");
		return;
	}

	if(rule->wasForwardDeclaration) {
		fprintf(out, "Forward ");
	}

	switch(rule->ruleType) {
		case PARSE_RULE_ALPHABET: {
			AlphabetParseRule* alphabetRule = rule->alphabetRule;
			fprintf(out, "Alphabet(\"%s\")", alphabetRule->alphabet);
			break;
		}
		case PARSE_RULE_OPTION_LIST: {
			fprintf(out, "Options");
			printRulesListRule(out, rule->optionListRule, maxDepth, depth);
			break;
		}
		case PARSE_RULE_SEQUENCE: {
			fprintf(out, "Sequence");
			printRulesListRule(out, rule->sequenceRule, maxDepth, depth);
			break;
		}
		case PARSE_RULE_FORWARD_DECLARED: {
			fprintf(out, "Unresolved Forward Declaration (%p)", rule);
			break;
		}
		case PARSE_RULE_STRING: {
			fprintf(out, "String(\"%s\")", rule->stringRule->string);
			break;
		}
		default: {
			fprintf(out, "Unknown Rule Type\n");
			break;
		}
	}
}

void printScheme(FILE* out, ParseScheme* scheme) {
	if(scheme == NULL) {
		fprintf(out, "Scheme is null!\n");
		return;
	}
	if(scheme->errorState != 0) {
		fprintf(out, "Scheme has error state %d!\n", scheme->errorState);
		return;
	}

	fprintf(out, "Scheme has %lu/%lu rule slots filled.\n", scheme->numRules, scheme->maxRules);
	if(scheme->numUnresolvedForwardRules > 0) {
		fprintf(out, "Scheme has %lu unresolved forward-declared rules.\n", scheme->numUnresolvedForwardRules);
	}

	for(size_t i = 0; i < scheme->numRules; i++)	{
		ParseRule* rule = scheme->rules + i;

		// fprintf(out, "| %p:\t", rule);

		Rule_Print(out, rule, 1, 1);
		fprintf(out, "\n");
	}
}

void printSchemeStandard(ParseScheme* scheme) {
	printScheme(stdout, scheme);
}

// PARSE_RULE_ALPHABET,
// 	PARSE_RULE_OPTION_LIST,
// 	PARSE_RULE_SEQUENCE,
// 	PARSE_RULE_FORWARD_DECLARED,
// 	PARSE_RULE_STRING
// void unitTests() {
// 	ParseScheme* scheme = ParseScheme_Create();

// 	ParseRule* base10Digit = Rule_Alphabet(scheme, "0123456789");

// 	ParseRule* 
// } 

int main(int argc, char** argv) {
	ParseScheme* scheme = ParseScheme_Create();

	ParseRule* abcs = Rule_Alphabet(scheme, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
	ParseRule* numbers = Rule_Alphabet(scheme, "0123456789");
	ParseRule* alphanumericRule = Rule_OptionList(scheme, abcs, numbers, NULL);
	ParseRule* codeFormat = Rule_Sequence(scheme, alphanumericRule, numbers, abcs, NULL);
	ParseRule* stringFormat = Rule_ForwardDeclare(scheme);
	Rule_SetForwardRuleValue(scheme, stringFormat, Rule_OptionList(scheme,
		Rule_Sequence(scheme, abcs, stringFormat, NULL),
		abcs
	));

	if(argc > 1) {
		ParseResult result;
		parseWithRule(argv[1], &result, stringFormat);
		printf("Result: %s, %lu\n", result.success? "success" : "failure", result.length);
	}

	printSchemeStandard(scheme);

	ParseScheme_Free(scheme);
	free(scheme);

	return 0;
}

