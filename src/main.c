#include <stdio.h>
#include <stdlib.h>
#include "ParseFramework.h"

int main(int argc, char** argv) {
	ParseScheme* scheme = ParseScheme_Create();

	ParseRule* base10Digit = AlphabetRule_Create(scheme, "0123456789");
	ParseRule* base10UnsignedIntegerLiteral = ForwardRule_Declare(scheme);
	ForwardRule_SetValue(scheme, base10UnsignedIntegerLiteral, OptionListRule_Create(scheme,
		SequenceRule_Create(scheme, base10Digit, base10UnsignedIntegerLiteral),
		base10Digit
	));

	ParseRule* unsignedIntegerExponentialLiteral = SequenceRule_Create(scheme,
		base10UnsignedIntegerLiteral,
		AlphabetRule_Create(scheme, "eE"),
		base10UnsignedIntegerLiteral
	);

	ParseRule* base16Digit = AlphabetRule_Create(scheme, "0123456789ABCDEFabcdef");
	ParseRule* base16SignlessInteger = ForwardRule_Declare(scheme);
	ForwardRule_SetValue(scheme, base16SignlessInteger, OptionListRule_Create(scheme, 
		SequenceRule_Create(scheme, base16Digit, base16SignlessInteger),
		base16Digit
	));
	ParseRule* base16UnsignedIntegerLiteral = SequenceRule_Create(scheme, 
		StringRule_Create(scheme, "0x"),
		base16SignlessInteger
	);

	ParseRule* base2Digit = AlphabetRule_Create(scheme, "01");
	ParseRule* base2SignlessInteger = ForwardRule_Declare(scheme);
	ForwardRule_SetValue(scheme, base2SignlessInteger, OptionListRule_Create(scheme,
		SequenceRule_Create(scheme, base2Digit, base2SignlessInteger),
		base2SignlessInteger
	));
	ParseRule* base2UnsignedIntegerLiteral = SequenceRule_Create(scheme,
		StringRule_Create(scheme, "0b"),
		base2SignlessInteger
	);
	
	ParseRule* unsignedIntegerLiteral = OptionListRule_Create(scheme,
		base10UnsignedIntegerLiteral,
		unsignedIntegerExponentialLiteral,
		base16UnsignedIntegerLiteral,
		base2UnsignedIntegerLiteral
	);

	ParseRule* integerLiteral = SequenceRule_Create(scheme,
		OptionalRule_Create(scheme, AlphabetRule_Create(scheme, "+-")),
		unsignedIntegerLiteral
	);

	ParseRule* listOfIntegers = ForwardRule_Declare(scheme);
	ForwardRule_SetValue(scheme, listOfIntegers, OptionListRule_Create(scheme, 
		SequenceRule_Create(scheme, integerLiteral, StringRule_Create(scheme, " "), listOfIntegers),
		integerLiteral
	));

	// ParseRule* floatFractionalPart = SequenceRule_Create(scheme, StringRule_Create(scheme, "."), base10UnsignedIntegerLiteral);
	// ParseRule* positiveFloat = OptionListRule_Create(
	// 	floatFractionalPart,
	// 	SequenceRule_Create(scheme, base10UnsignedIntegerLiteral, floatFractionalPart)
	// );



	// ParseRule* abcs = AlphabetRule_Create(scheme, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
	// ParseRule* numbers = AlphabetRule_Create(scheme, "0123456789");
	// ParseRule* alphanumericRule = OptionListRule_Create(scheme, abcs, numbers, NULL);
	// ParseRule* codeFormat = SequenceRule_Create(scheme, alphanumericRule, numbers, abcs, NULL);
	// ParseRule* stringFormat = ForwardRule_Declare(scheme);
	// ForwardRule_SetValue(scheme, stringFormat, OptionListRule_Create(scheme,
	// 	SequenceRule_Create(scheme, abcs, stringFormat, NULL),
	// 	abcs
	// ));

	if(argc > 1) {
		ParseResult result;
		// Rule_Parse(stringFormat, argv[1], &result);
		Rule_Parse(listOfIntegers, argv[1], &result);
		printf("Result: %s, %lu\n", result.success? "success" : "failure", result.length);
	}

	ParseScheme_Print(scheme, stdout);

	ParseScheme_Free(scheme);
	free(scheme);

	return 0;
}