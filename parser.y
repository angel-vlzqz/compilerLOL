%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

%token <string> TYPE_INT     // For "int"}
%token <string> TYPE_CHAR    // For "char"}
%token <string> WRITE        // For "write"}
%token <string> WHILE        // For "while"}
%token <string> RETURN       // For "return"}
%token <string> IF           // For "if"}
%token <string> ELSE         // For "else"}
%token <string> ID           // For identifiers}
%token <number> NUMBER       // For numbers}
// %token <string> THEN         // For "then"
// %token <string> DO           // For "do"
%token <character> SEMICOLON // For ';'}
%token <operator> ASSIGNOP   // For '='}
%token <operator> PLUS       // For '+'}
%token <operator> MINUS      // For '-'}
%token <operator> MUL        // For '*'}
%token <operator> LOGICOP    // For logical operators (==, &&, ||, <, <=, >, >=, !=)}
%token '('                   // For '('}
%token ')'                   // For ')'}
%token '['                   // For '['}
%token ']'                   // For ']'}
%token '{'                   // For '{'}
%token '}'                   // For '}'}

%left PLUS MINUS
%left MUL

%type <ast> Program VarDecl VarDeclList Stmt StmtList Expr BinOp
%type <operator> Op
%type <string> Type // fixes $1 and $2 to be of type string
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
    Type ID SEMICOLON {
        $$ = createNode(NodeType_VarDecl);
        $$->varDecl.varType = strdup($1);
        $$->varDecl.varName = strdup($2);
        // Insert into symbol table.
    } | Type ID {
        printf("Missing semicolon after declaring variable: %s\n", $2);
    }
    ;

Type:
    TYPE_INT {
        $$ = createNode(NodeType_Type);  // Return an ASTNode*
        $$->typeNode.type = strdup($1);  // Set the type to int
    }
    | TYPE_CHAR {
        $$ = createNode(NodeType_Type);  // Return an ASTNode*
        $$->typeNode.type = strdup($1);  // Set the type to char
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
    | WRITE ID {
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
    // | IF Expr THEN Block ELSE Block {
    //     printf("Parsed If-Else Statement\n");
    // }
    // | WHILE Expr DO Block {
    //     printf("Parsed While Statement\n");
    // }
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
        $$->logicalOp.logicalOp = strdup($2);
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

BinOp:
    PLUS    { $$ = $1; }
    | MINUS { $$ = $1; }
    | MUL   { $$ = $1; }
    ;

// Stmt:
//     ID ASSIGNOP Expr SEMICOLON {
//         printf("Parsed Assignment Statement: %s\n", $1);
//     }
//     | WRITE ID {
//         printf("Parsed Write Statement: %s\n", $2);
//     }
//     ;

//xpr:
//   ID Op ID {
//       printf("Parsed Expression: %s + %s\n", $1, $3);
//   }
//   | ID Op NUMBER {
//       printf("Parsed Expression: %s + %s\n", $1, $3);
//   }
//   | NUMBER Op ID {
//       printf("Parsed Expression: %s + %s\n", $1, $3);
//   }
//   | NUMBER Op NUMBER {
//       printf("Parsed Expression: %s + %s\n", $1, $3);
//   }
//   | ID {
//       printf("Parsed Identifier: %s\n", $1);
//   }
//   | NUMBER {
//       printf("Parsed Number: %s\n", $1);
//   }
//   ;
    Op: PLUS {
        { $$ = $1; }
        printf("Parsed Operator: +\n");
    }
    | MINUS {
        { $$ = $1; }
       printf("Parsed Operator: -\n");
    }
   | MUL {
        { $$ = $1; }
        printf("Parsed Operator: *\n");
   }
   ;

%% 

void yyerror(const char *s) {
    printf("Error: %s\n", s);
    exit(1);
}

int main() {
    // check if file is provided
    yyin = fopen("test_all_tokens.c", "r");
    if (!yyin) {
        fprintf(stderr, "Error: Unable to open file\n");
        exit(1);
    }
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

    // Traverse and print the AST
    if (root != NULL) {
        traverseAST(root, 0);
        freeAST(root);
    }

    freeSymbolTable(symTab);
    return 0;
}

// void yyerror(const char* s) {
// 	fprintf(stderr, "Parse error: %s\n", s);
// 	exit(1);
// }
