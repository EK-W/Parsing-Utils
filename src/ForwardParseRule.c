#include <stdio.h>
#include "ParseFramework.h"

ParseRule* ForwardRule_Declare(ParseScheme* scheme) {
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

ParseRule* ForwardRule_SetValue(ParseScheme* scheme, ParseRule* forwardRule, ParseRule* ruleValue) {
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

void ForwardRule_Print(ParseRule* rule, FILE* fout) {
	fprintf(fout, "Unresolved Forward Declaration (%p)", rule);
}