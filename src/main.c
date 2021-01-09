#include <stdio.h>
#include <stdlib.h>
#include "ParseFramework.h"

int main(int argc, char** argv) {
	ParseScheme* scheme = ParseScheme_Create();

	ParseRule* abcs = AlphabetRule_Create(scheme, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
	ParseRule* numbers = AlphabetRule_Create(scheme, "0123456789");
	ParseRule* alphanumericRule = OptionListRule_Create(scheme, abcs, numbers, NULL);
	ParseRule* codeFormat = SequenceRule_Create(scheme, alphanumericRule, numbers, abcs, NULL);
	ParseRule* stringFormat = ForwardRule_Declare(scheme);
	ForwardRule_SetValue(scheme, stringFormat, OptionListRule_Create(scheme,
		SequenceRule_Create(scheme, abcs, stringFormat, NULL),
		abcs
	));

	if(argc > 1) {
		ParseResult result;
		Rule_Parse(stringFormat, argv[1], &result);
		printf("Result: %s, %lu\n", result.success? "success" : "failure", result.length);
	}

	ParseScheme_Print(scheme, stdout);

	ParseScheme_Free(scheme);
	free(scheme);

	return 0;
}