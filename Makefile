# Makefile

# Compiler and flags
CC = gcc
CFLAGS = -Wall -g -Wextra -Werror
LDFLAGS = -g
BISON = bison
FLEX = flex

# Executable
EXEC = main_program

# Files
BISON_SRC = parser.y
FLEX_SRC = lexer.l
BISON_OUTPUT = parser.tab.c
FLEX_OUTPUT = lex.yy.c
OBJS = parser.tab.o lex.yy.o AST.o SymbolTable.o semantic.o optimizer.o codeGenerator.o Array.o

# Default rule to build the executable
all: $(EXEC)

# Build the executable by linking all object files
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS)

# Compile Bison file
parser.tab.o: $(BISON_SRC)
	$(BISON) -d $(BISON_SRC) -o $(BISON_OUTPUT)
	$(CC) $(CFLAGS) -c $(BISON_OUTPUT) -o parser.tab.o -w

# Compile Flex file
lex.yy.o: $(FLEX_SRC) parser.tab.h
	$(FLEX) $(FLEX_SRC)
	$(CC) $(CFLAGS) -c $(FLEX_OUTPUT) -o lex.yy.o -w

# Compile AST.c
AST.o: AST.c AST.h
	$(CC) $(CFLAGS) -c AST.c -o AST.o -w

# Compile SymbolTable.c
SymbolTable.o: SymbolTable.c SymbolTable.h Array.h
	$(CC) $(CFLAGS) -c SymbolTable.c -o SymbolTable.o -w

# Compile Semantic Analysis
semantic.o: semantic.c semantic.h AST.h SymbolTable.h Array.h
	$(CC) $(CFLAGS) -c semantic.c -o semantic.o -w

# Compile Optimizer
optimizer.o: optimizer.c optimizer.h semantic.h
	$(CC) $(CFLAGS) -c optimizer.c -o optimizer.o -w

# Compile Code Generator
codeGenerator.o: codeGenerator.c codeGenerator.h AST.h semantic.h Array.h
	$(CC) $(CFLAGS) -c codeGenerator.c -o codeGenerator.o -w

# Compile Array.c
Array.o: Array.c Array.h
	$(CC) $(CFLAGS) -c Array.c -o Array.o -w

# Clean rule to remove all generated files
clean:
	rm -f $(OBJS) $(EXEC) $(BISON_OUTPUT) parser.tab.h $(FLEX_OUTPUT) semantic.o optimizer.o codeGenerator.o Array.o TACgen.ir TACopt.ir Tacsem.ir
