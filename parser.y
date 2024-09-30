%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AST.h"
#include "SymbolTable.h"
#include "semantic.h"
#include "optimizer.h"
#include "codeGenerator.h"

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
         
%token <number> NUMBER       
%token <string> IF ELSE WHILE RETURN WRITE ID TYPE
%token <operator> ASSIGNOP PLUS MINUS MUL LOGICOP
%token <character> SEMICOLON '(' ')' '[' ']' '{' '}'
%token THEN DO

%left PLUS MINUS
%left MUL

%type <ast> Program VarDeclList VarDecl Stmt StmtList Expr Block

%start Program

%% 

Program:
    VarDeclList Block {
        printf("Parsed Program\n");
	    root = malloc(sizeof(ASTNode));
        root->type = NodeType_Program;
        root->program.varDeclList = $1;
        root->program.stmtList = $2;
    }
    ;

VarDeclList:
    VarDecl VarDeclList {
        // Handle recursive variable declaration list.
        $$ = createNode(NodeType_VarDeclList);
        $$->varDeclList.varDecl = $1;
        $$->varDeclList.varDeclList = $2;
    }
    | VarDecl {
        // Handle single variable declaration.
        $$ = createNode(NodeType_VarDeclList);
        $$->varDeclList.varDecl = $1;
        $$->varDeclList.varDeclList = NULL;
    }
    ;

VarDecl:
    TYPE ID SEMICOLON {
        $$ = createNode(NodeType_VarDecl);
        $$->varDecl.varType = strdup($1);
        $$->varDecl.varName = strdup($2);
        // Insert into symbol table.
        insertSymbol(symTab, $2, $1);
    } | TYPE ID {
        printf("Missing semicolon after declaring variable: %s\n", $2);
    }
    ;

Block:
    StmtList {
        printf("Parsed Block\n");
	$$ = createNode(NodeType_Block);
        $$->block.stmtList = $1;
    }
    ;

StmtList:
    Stmt StmtList {
        printf("Parsed Statement List\n");
	    $$ = malloc(sizeof(ASTNode));
        $$->type = NodeType_StmtList;
        $$->stmtList.stmt = $1;
        $$->stmtList.stmtList = $2;
    }
    | /* empty */ {
        printf("Parsed Empty Statement List\n");
    }
    ;

Stmt:
    ID ASSIGNOP Expr SEMICOLON {
        Symbol* existingSymbol = findSymbol(symTab, $1);
        if (existingSymbol != NULL) {
            printf("Parsed Assignment Statement: %s = ...\n", $1);

            // Use a local buffer to store the value string
            char valueBuffer[20];  // No need to free this

            if ($3->type == NodeType_SimpleExpr) {
                snprintf(valueBuffer, sizeof(valueBuffer), "%d", $3->simpleExpr.number);
            } else {
                // Handle other expression types
            }

            // Update the symbol with the extracted value
            updateSymbolValue(symTab, $1, valueBuffer);

            $$ = malloc(sizeof(ASTNode));
            $$->type = NodeType_AssignStmt;
            $$->assignStmt.varName = strdup($1);
            $$->assignStmt.operator = strdup($2);
            $$->assignStmt.expr = $3;
        } else {
            printf("Error: Variable %s not declared\n", $1);
            yyerror("Undeclared variable");
        }
    }
    | WRITE ID SEMICOLON {
        Symbol* existingSymbol = findSymbol(symTab, $2);
        if (existingSymbol != NULL) {
            printf("Parsed Write Statement: %s\n", $2);
            $$ = createNode(NodeType_WriteStmt);
            $$->writeStmt.varName = strdup($2);
        } else {
            printf("Error: Variable %s not declared\n", $2);
            yyerror("Undeclared variable");
        }
    }
    | IF Expr THEN Block ELSE Block {
        printf("Parsed If-Else Statement\n");
        $$ = createNode(NodeType_IfStmt);
        $$->ifStmt.condition = $2;
        $$->ifStmt.thenBlock = $4;
        $$->ifStmt.elseBlock = $6;
    }
    | WHILE Expr DO Block {
        printf("Parsed While Statement\n");
        $$ = createNode(NodeType_WhileStmt);
        $$->whileStmt.condition = $2;
        $$->whileStmt.block = $4;
    }
    | RETURN Expr SEMICOLON {
        printf("Parsed Return Statement\n");
	    $$ = createNode(NodeType_ReturnStmt);
        $$->returnStmt.expr = $2;
    }
    ;

Expr:
    Expr PLUS Expr {
        printf("PARSER: Recognized addition expression\n");
        $$ = malloc(sizeof(ASTNode));
        $$->type = NodeType_BinOp;
        $$->binOp.operator = '+';
        $$->binOp.left = $1;
        $$->binOp.right = $3;
    }
    | Expr MINUS Expr {
        printf("PARSER: Recognized subtraction expression\n");
        $$ = malloc(sizeof(ASTNode));
        $$->type = NodeType_BinOp;
        $$->binOp.operator = '-';
        $$->binOp.left = $1;
        $$->binOp.right = $3;
    }
    | Expr MUL Expr {
        printf("PARSER: Recognized multiplication expression\n");
        $$ = malloc(sizeof(ASTNode));
        $$->type = NodeType_BinOp;
        $$->binOp.operator = '*';
        $$->binOp.left = $1;
        $$->binOp.right = $3;
    }
    | Expr LOGICOP Expr {
        printf("Parsed Logical Expression: %s %s %s\n", $1, $2, $3);
        $$ = createNode(NodeType_LogicalOp);
        $$->logicalOp.logicalOp = strdup($2);  // Store the operator string
        $$->logicalOp.left = $1;
        $$->logicalOp.right = $3;
    }
    | '(' Expr ')' {
        printf("Parsed Expression in parentheses\n");
        $$ = $2;
    }
    | ID {
        Symbol* existingSymbol = findSymbol(symTab, $1);
        if (existingSymbol != NULL) {
            printf("Parsed Identifier: %s\n", $1);
	        $$ = malloc(sizeof(ASTNode));
	        $$->type = NodeType_SimpleID;
	        $$->simpleID.name = $1;
        } else {
            printf("Error: Variable %s not declared\n", $1);
            yyerror("Undeclared variable");
        }
    } 
    | NUMBER {
        printf("Parsed Number: %d\n", $1);
	    $$ = malloc(sizeof(ASTNode));
	    $$->type = NodeType_SimpleExpr;
	    $$->simpleExpr.number = $1;
    }
    ;


%% 

void yyerror(const char *s) {
    printf("Error: %s\n", s);
    exit(1);
}

int main() {
    // initialize the input source
    yyin = fopen("input.cmm", "r");

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

    initializeTempVars();
    
    if (yyparse() == 0) {

        // Semantic Analysis
        semanticAnalysis(root, symTab);

        // TAC Optimization
        optimizeTAC(&tacHead);  // 'tacHead' is the global head of the TAC linked list

        // Code Generation
        initCodeGenerator("output.asm");
        generateMIPS(tacHead);  // Generate MIPS code from optimized TAC
        finalizeCodeGenerator("output.asm");
    }

    // Traverse and print the AST
    if (root != NULL) {
        traverseAST(root, 0);
        freeAST(root);
    }

    freeSymbolTable(symTab);
    fclose(yyin);
    return 0;
}
