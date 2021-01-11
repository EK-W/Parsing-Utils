#ifndef EKW_PARSER_PARSE_FRAMEWORK_H
#define EKW_PARSER_PARSE_FRAMEWORK_H

#include <stdio.h>
#include <stdbool.h>


typedef struct ParseRule_s ParseRule;


// =================
// Rule type defs...
// =================

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

typedef struct {
	char* string;
	size_t stringLen;
} StringParseRule;

typedef struct {
	ParseRule* rule;
	// bool greedy;
} OptionalParseRule;


// =================================
// Other defs...
// =================================

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


typedef enum {
	PARSE_RULE_NO_TYPE,
	PARSE_RULE_ALPHABET,
	PARSE_RULE_OPTION_LIST,
	PARSE_RULE_SEQUENCE,
	PARSE_RULE_FORWARD_DECLARED,
	PARSE_RULE_STRING,
	PARSE_RULE_OPTIONAL
} ParseRuleType;

struct ParseRule_s {
	// It's generally a good idea to only set a new rule's type *after* you have allocated its data.
	// This will help avoid bugs where we try to free data that hasn't been allocated
	ParseRuleType ruleType;

	bool wasForwardDeclaration;

	ParseScheme* scheme;

	union {
		AlphabetParseRule* alphabetRule;
		OptionListParseRule* optionListRule;
		SequenceParseRule* sequenceRule;
		StringParseRule* stringRule;
		OptionalParseRule* optionalRule;
	};
};





typedef struct {
	bool success;
	char* str;
	size_t length;
} ParseResult;

//extern ParseResult PARSE_RESULT_FAILURE;

// ======================
// functions...
// ======================

ParseScheme* ParseScheme_Create();
void ParseScheme_Free(ParseScheme* scheme);
ParseRule* getSchemeSpaceForNewRule(ParseScheme* scheme);
void ParseScheme_Print(ParseScheme* scheme, FILE* fout);

void Rule_Free(ParseRule* rule);

ParseResult Rule_Parse(ParseRule* rule, char* str, ParseResult* result_ret);

void Rule_PrintSimpleRulePointer(ParseRule* rule, FILE* fout);
void Rule_PrintDeep(ParseRule* rule, FILE* fout, size_t depth, size_t maxDepth, char* indentStr);
void Rule_Print(ParseRule* rule, FILE* fout);

ParseResult setParseResult(ParseResult* ptr, bool success, char* str, size_t length);
//ParseResult useParseResult(ParseResult* ptr, ParseResult toUse);

// ==========================
// Rule creation functions...
// ==========================

#define OptionListRule_Create(scheme, ...) createOptionListRule(scheme, __VA_ARGS__, NULL)
#define SequenceRule_Create(scheme, ...) createSequenceRule(scheme, __VA_ARGS__, NULL)

ParseRule* AlphabetRule_Create(ParseScheme* scheme, char* str);
ParseRule* ForwardRule_Declare(ParseScheme* scheme);
ParseRule* ForwardRule_SetValue(ParseScheme* scheme, ParseRule* forwardRule, ParseRule* ruleValue);
ParseRule* createOptionListRule(ParseScheme* scheme, ...);
ParseRule* createSequenceRule(ParseScheme* scheme, ...);
ParseRule* StringRule_Create(ParseScheme* scheme, char* str);
ParseRule* OptionalRule_Create(ParseScheme* scheme, ParseRule* rule);

#endif