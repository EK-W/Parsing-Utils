FILENAMES = $(addprefix src/,AlphabetParseRule OptionListParseRule ParseFramework RulesListRuleUtil SequenceParseRule StringParseRule ForwardParseRule)
HEADERS = $(addsuffix .h,$(FILENAMES))
CFILES = $(addsuffix .c,$(FILENAMES))

main: $(HEADERS) $(CFILES) src/main.c
	gcc -o main src/main.c $(CFILES)
