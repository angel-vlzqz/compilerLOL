# Makefile

# Compiler and flags
CC = gcc
CFLAGS = -Wall -g
BISON = bison
FLEX = flex
EXEC = main_program

# Files
BISON_SRC = parser.y
FLEX_SRC = lexer.l
BISON_OUTPUT = parser.tab.c
FLEX_OUTPUT = lex.yy.c
OBJS = parser.tab.o lex.yy.o AST.o SymbolTable.o

# Default rule to build the executable
all: $(EXEC)

# Build the executable by linking all object files
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS)

# Compile Bison file
parser.tab.o: $(BISON_SRC)
	$(BISON) -d $(BISON_SRC) -o $(BISON_OUTPUT)
	$(CC) $(CFLAGS) -c $(BISON_OUTPUT) -o parser.tab.o

# Compile Flex file
lex.yy.o: $(FLEX_SRC) parser.tab.h
	$(FLEX) $(FLEX_SRC)
	$(CC) $(CFLAGS) -c $(FLEX_OUTPUT) -o lex.yy.o

# Compile AST.c
AST.o: AST.c AST.h
	$(CC) $(CFLAGS) -c AST.c -o AST.o

# Compile SymbolTable.c
SymbolTable.o: SymbolTable.c SymbolTable.h
	$(CC) $(CFLAGS) -c SymbolTable.c -o SymbolTable.o

# Clean rule to remove all generated files
clean:
	rm -f $(OBJS) $(EXEC) $(BISON_OUTPUT) parser.tab.h $(FLEX_OUTPUT)
