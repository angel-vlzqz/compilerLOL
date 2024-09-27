# Makefile

# Compiler and flags
CC = gcc
BISON = bison
FLEX = flex
OUTPUT = my_compiler

# Files
LEXER = lexer.l
PARSER = parser.y
INPUT = test_all_tokens.c
BISON_OUTPUT = parser.tab.c parser.tab.h
LEXER_OUTPUT = lex.yy.c
SYMBOL_TABLE = SymbolTable.c

# Default target: compile and run
all: $(OUTPUT)
	./$(OUTPUT) < $(INPUT)

# Generate the parser files
parser.tab.c: $(PARSER)
	$(BISON) -d $(PARSER)

# Generate the lexer file
lex.yy.c: $(LEXER)
	$(FLEX) $(LEXER)

# Compile and link the lexer, parser, and symbol table
$(OUTPUT): parser.tab.c lex.yy.c $(SYMBOL_TABLE)
	$(CC) -o $(OUTPUT) parser.tab.c lex.yy.c $(SYMBOL_TABLE) -ll -w

# Clean up generated files
clean:
	rm -f $(OUTPUT) $(LEXER_OUTPUT) $(BISON_OUTPUT) 