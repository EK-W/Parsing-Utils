#include <stdio.h>
#include <stdlib.h>
#include "ParseFramework.h"

int main(int argc, char** argv) {
	ParseScheme* scheme = ParseScheme_Create();

	ParseRule* base10Digit = AlphabetRule_Create(scheme, "0123456789");
	ParseRule* base10UnsignedIntegerLiteral = ForwardRule_Declare(scheme);
	ForwardRule_SetValue(scheme, base10UnsignedIntegerLiteral, SequenceRule_Create(scheme,
		base10Digit, OptionalRule_Create(scheme, base10UnsignedIntegerLiteral)
	));

	ParseRule* unsignedIntegerExponentialLiteral = SequenceRule_Create(scheme,
		base10UnsignedIntegerLiteral,
		AlphabetRule_Create(scheme, "eE"),
		base10UnsignedIntegerLiteral
	);

	ParseRule* base16Digit = AlphabetRule_Create(scheme, "0123456789ABCDEFabcdef");
	ParseRule* base16SignlessInteger = ForwardRule_Declare(scheme);
	ForwardRule_SetValue(scheme, base16SignlessInteger, SequenceRule_Create(scheme,
		base16Digit, OptionalRule_Create(scheme, base16SignlessInteger)
	));


	ParseRule* base16UnsignedIntegerLiteral = SequenceRule_Create(scheme, 
		StringRule_Create(scheme, "0x"),
		base16SignlessInteger
	);

	ParseRule* base2Digit = AlphabetRule_Create(scheme, "01");
	ParseRule* base2SignlessInteger = ForwardRule_Declare(scheme);
	ForwardRule_SetValue(scheme, base2SignlessInteger, SequenceRule_Create(scheme,
		base2Digit, OptionalRule_Create(scheme, base2SignlessInteger)
	));

	ParseRule* base2UnsignedIntegerLiteral = SequenceRule_Create(scheme,
		StringRule_Create(scheme, "0b"),
		base2SignlessInteger
	);
	
	ParseRule* unsignedIntegerLiteral = OptionListRule_Create(scheme,
		unsignedIntegerExponentialLiteral,
		base16UnsignedIntegerLiteral,
		base2UnsignedIntegerLiteral,
		base10UnsignedIntegerLiteral
	);

	ParseRule* integerLiteral = SequenceRule_Create(scheme,
		OptionalRule_Create(scheme, AlphabetRule_Create(scheme, "+-")),
		unsignedIntegerLiteral
	);

	ParseRule* listOfIntegers = ForwardRule_Declare(scheme);
	ForwardRule_SetValue(scheme, listOfIntegers, SequenceRule_Create(scheme,
		integerLiteral, OptionalRule_Create(scheme, SequenceRule_Create(scheme,
			StringRule_Create(scheme, " "), listOfIntegers
		))
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

	ParseScheme_Print(scheme, stdout);
	printf("\n=======\n\n");

	if(argc > 1) {
		ParseResult result;
		// Rule_Parse(stringFormat, argv[1], &result);
		Rule_Parse(listOfIntegers, argv[1], &result);
		printf("Result: %s, %lu\n", result.success? "success" : "failure", result.length);
	}

	

	ParseScheme_Free(scheme);
	free(scheme);

	return 0;
}