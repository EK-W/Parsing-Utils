#ifndef EKW_PARSER_FORWARD_PARSE_RULE_H
#define EKW_PARSER_FORWARD_PARSE_RULE_H

#include <stdio.h>
#include "ParseFramework.h"

ParseRule* ForwardRule_Declare(ParseScheme* scheme);

ParseRule* ForwardRule_SetValue(ParseScheme* scheme, ParseRule* forwardRule, ParseRule* ruleValue);

void ForwardRule_Print(ParseRule* rule, FILE* fout);


#endif