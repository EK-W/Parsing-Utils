FILENAMES = AlphabetParseRule OptionListParseRule ParseFramework RulesListRuleUtil SequenceParseRule StringParseRule ForwardParseRule OptionalParseRule RepeatParseRule
HEADERS = $(addprefix src/headers/,$(addsuffix .h,$(FILENAMES)))
CFILES = $(addprefix src/,$(addsuffix .c,$(FILENAMES)))

main: $(HEADERS) $(CFILES) src/main.c
	gcc -o main src/main.c $(CFILES) -g -I'src/headers/'
