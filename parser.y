%{
#include <stdio.h>
#include <stdlib.h>
#include "AST.h"
#include "SymbolTable.h"

#define TABLE_SIZE 101


extern int yylex();
extern FILE* yyin;
extern int yylineno;

void yyerror(const char *s);

ASTNode* root = NULL;
SymbolTable* symTab = NULL;
Symbol* symbol = NULL;

%}

%union {
	int number;
	char character;
	char* string;
	char* operator;
	struct ASTNode* ast;
}

%token <string> TYPE_INT     {// For "int"}
%token <string> TYPE_CHAR    {// For "char"}
%token <string> WRITE        {// For "write"}
%token <string> WHILE        {// For "while"}
%token <string> RETURN       {// For "return"}
%token <string> IF           {// For "if"}
%token <string> ELSE         {// For "else"}
%token <string> ID           {// For identifiers}
%token <number> NUMBER       {// For numbers}
%token <character> SEMICOLON {// For ';'}
%token <operator> ASSIGNOP   {// For '='}
%token <operator> PLUS       {// For '+'}
%token <operator> MINUS      {// For '-'}
%token <operator> MUL        {// For '*'}
%token <operator> LOGICOP    {// For logical operators (==, &&, ||, <, <=, >, >=, !=)}
%token '('                   {// For '('}
%token ')'                   {// For ')'}
%token '['                   {// For '['}
%token ']'                   {// For ']'}
%token '{'                   {// For '{'}
%token '}'                   {// For '}'}

%left PLUS MINUS
%left MUL

%type <ast> Program VarDecl VarDeclList Stmt StmtList Expr BinOp
%start Program

%% 

Program:
    VarDecl Block {
        printf("Parsed Program\n");
    }
    ;


VarDecl:
    Type ID SEMICOLON VarDecl {
        // Insert variable into symbol table
        Symbol* existingSymbol = findSymbol(symTab, $2);
        if (existingSymbol == NULL) {
            insertSymbol(symTab, $2, $1);
            printf("Declared Variable: %s of type %s\n", $2, $1);
        } else {
            printf("Error: Variable %s already declared\n", $2);
            yyerror("Variable redeclaration");
        }
    }
    | /* empty */ {
        // No variables declared
    }
    ;

Stmt:
    ID ASSIGNOP Expr SEMICOLON {
        Symbol* existingSymbol = findSymbol(symTab, $1);
        if (existingSymbol != NULL) {
            printf("Parsed Assignment Statement: %s = ...\n", $1);
            // Here we could update the symbol value if needed
        } else {
            printf("Error: Variable %s not declared\n", $1);
            yyerror("Undeclared variable");
        }
    }
    | WRITE ID {
        Symbol* existingSymbol = findSymbol(symTab, $2);
        if (existingSymbol != NULL) {
            printf("Parsed Write Statement: %s\n", $2);
        } else {
            printf("Error: Variable %s not declared\n", $2);
            yyerror("Undeclared variable");
        }
    }
    | IF Expr THEN Block ELSE Block {
        printf("Parsed If-Else Statement\n");
    }
    | WHILE Expr DO Block {
        printf("Parsed While Statement\n");
    }
    | RETURN Expr SEMICOLON {
        printf("Parsed Return Statement\n");
    }
    ;

Expr:
    ID {
        Symbol* existingSymbol = findSymbol(symTab, $1);
        if (existingSymbol != NULL) {
            printf("Parsed Identifier: %s\n", $1);
        } else {
            printf("Error: Variable %s not declared\n", $1);
            yyerror("Undeclared variable");
        }
    }
    | NUMBER {
        printf("Parsed Number: %s\n", $1);
    }
    | Expr Op Expr {
        printf("Parsed Expression: +\n");
    }
    | Expr LOGICOP Expr {
        printf("Parsed Expression: <\n");
    }
    | '(' Expr ')' {
        printf("Parsed Expression in parentheses\n");
    }
    ;


Type:
    TYPE_INT {
        printf("Parsed Type: int\n");
    }
    | TYPE_CHAR {
        printf("Parsed Type: char\n");
    }
    ;

Block:
    StmtList {
        printf("Parsed Block\n");
    }
    ;

StmtList:
    Stmt StmtList {
        printf("Parsed Statement List\n");
    }
    | /* empty */ {
        printf("Parsed Empty Statement List\n");
    }
    ;

Stmt:
    ID ASSIGNOP Expr SEMICOLON {
        printf("Parsed Assignment Statement: %s\n", $1);
    }
    | WRITE ID {
        printf("Parsed Write Statement: %s\n", $2);
    }
    ;

Expr:
    ID Op ID {
        printf("Parsed Expression: %s + %s\n", $1, $3);
    }
    | ID Op NUMBER {
        printf("Parsed Expression: %s + %s\n", $1, $3);
    }
    | NUMBER Op ID {
        printf("Parsed Expression: %s + %s\n", $1, $3);
    }
    | NUMBER Op NUMBER {
        printf("Parsed Expression: %s + %s\n", $1, $3);
    }
    | ID {
        printf("Parsed Identifier: %s\n", $1);
    }
    | NUMBER {
        printf("Parsed Number: %s\n", $1);
    }
    ;
    Op: PLUS {
        printf("Parsed Operator: +\n");
    }
    | MINUS {
        printf("Parsed Operator: -\n");
    }
    | MUL {
        printf("Parsed Operator: *\n");
    }
    ;

%% 

void yyerror(const char *s) {
    printf("Error: %s\n", s);
    exit(1);
}

int main() {
    // initialize the input source
    yyin = fopen("test_all_tokens.c", "r");

    // initialize symbol table
    symTab = createSymbolTable(TABLE_SIZE);
    if (symTab == NULL) {
        fprintf(stderr, "Error: Unable to initialize symbol table\n");
        exit(1);
    }
    symbol = malloc(sizeof(Symbol));
    if (symbol == NULL) {
        fprintf(stderr, "Error: Unable to allocate memory for symbol\n");
        exit(1);
    }
    yyparse();
    freeSymbolTable(symTab);
    return 0;
}

void yyerror(const char* s) {
	fprintf(stderr, "Parse error: %s\n", s);
	exit(1);
}
