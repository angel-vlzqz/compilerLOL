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

%}

%union 
{
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
%token THEN DO TRUE FALSE

%left '+' '-'
%left '*' '/'
%nonassoc UMINUS

%left PLUS MINUS
%left MUL

%type <ast> Program VarDeclList VarDecl Stmt StmtList Expr Block

%start Program

%% 

Program:
   VarDeclList Block 
    {
        printf("Parsed Program\n");
        root = malloc(sizeof(ASTNode));
        root->type = NodeType_Program;
        root->program.varDeclList = $1;
        root->program.block = $2;
    }
    ;
// FuncDeclList:
//     FuncDecl
//     | FuncDeclList FuncDecl
//     ;
// 
// FuncDecl:
//     TYPE ID '(' ParamList ')' '{' StmtList '}' {
//         /* Code to handle function declaration, create a symbol for the function */
//         printf("Function Declaration: %s\n", $2);
//     }
//     ;

//ParamList:
//    /* Handles parameters in the function */
//    TYPE ID {
//        /* Process single parameter */
//    }
//    | ParamList ',' TYPE ID {
//        /* Process multiple parameters */
//    }
//    | /* empty */ {
//        /* No parameters */
//    }
//    ;

    statement:
        Expr ';'
        | RETURN Expr ';' {
            /* Handle return statement */
            printf("Returning value from function.\n");
        }
        | WRITE '(' Expr ')' ';' {
            /* Handle write statement */
            printf("Writing value: %d\n", $3);
        }
        ;

    argument_list:
    //Expr
    //    | argument_list ',' Expr
    //    | /* empty */
    //    ;    
    
    

VarDeclList:
    VarDecl VarDeclList 
    {
        // Handle recursive variable declaration list.
        $$ = createNode(NodeType_VarDeclList);
        $$->varDeclList.varDecl = $1;
        $$->varDeclList.varDeclList = $2;
    }
    | VarDecl 
    {
        // Handle single variable declaration.
        $$ = createNode(NodeType_VarDeclList);
        $$->varDeclList.varDecl = $1;
        $$->varDeclList.varDeclList = NULL;
    }
    ;

VarDecl:
    TYPE ID SEMICOLON
    {
        $$ = createNode(NodeType_VarDecl);
        $$->varDecl.varType = strdup($1);
        $$->varDecl.varName = strdup($2);
    }
    | TYPE ID '[' NUMBER ']' SEMICOLON
    {
        $$ = createNode(NodeType_ArrayDecl);
        $$->arrayDecl.varType = strdup($1);
        $$->arrayDecl.varName = strdup($2);
        $$->arrayDecl.size = $4; // $4 is NUMBER
    }
    | TYPE ID
    {
        printf("Missing semicolon after declaring variable: %s\n", $2);
    }
    ;

Block:
    StmtList 
    {
        printf("Parsed Block\n");
        $$ = createNode(NodeType_Block);
        $$->block.stmtList = $1;
    }
    ;

StmtList:
    Stmt StmtList 
    {
        printf("Parsed Statement List\n");
        $$ = malloc(sizeof(ASTNode));
        $$->type = NodeType_StmtList;
        $$->stmtList.stmt = $1;
        $$->stmtList.stmtList = $2;
    }
    | /* empty */ 
    {
        printf("Parsed Empty Statement List\n");
        $$ = NULL;
    }
    ;

Stmt:
    ID ASSIGNOP Expr SEMICOLON 
    {
        printf("Parsed Assignment Statement: %s = ...\n", $1);

        $$ = malloc(sizeof(ASTNode));
        $$->type = NodeType_AssignStmt;
        $$->assignStmt.varName = strdup($1);
        $$->assignStmt.operator = strdup($2);
        $$->assignStmt.expr = $3;
    }
    | ID '[' Expr ']' ASSIGNOP Expr SEMICOLON
    {
        printf("Parsed Array Assignment: %s[%s] = ...\n", $1, $3);
        $$ = createNode(NodeType_ArrayAssign);
        $$->arrayAssign.arrayName = strdup($1);
        $$->arrayAssign.index = $3;
        $$->arrayAssign.expr = $6;
    }
    | WRITE Expr SEMICOLON 
    {
        printf("Parsed Write Statement: %s\n", $2);
        $$ = createNode(NodeType_WriteStmt);
        $$->writeStmt.expr = $2;
    }
    | IF Expr THEN Block ELSE Block 
    {
        printf("Parsed If-Else Statement\n");
        $$ = createNode(NodeType_IfStmt);
        $$->ifStmt.condition = $2;
        $$->ifStmt.thenBlock = $4;
        $$->ifStmt.elseBlock = $6;
    }
    | WHILE Expr DO Block 
    {
        printf("Parsed While Statement\n");
        $$ = createNode(NodeType_WhileStmt);
        $$->whileStmt.condition = $2;
        $$->whileStmt.block = $4;
    }
    | RETURN Expr SEMICOLON 
    {
        printf("Parsed Return Statement\n");
        $$ = createNode(NodeType_ReturnStmt);
        $$->returnStmt.expr = $2;
    }
    ;

Expr:
    Expr PLUS Expr 
    {
        printf("PARSER: Recognized addition expression\n");
        $$ = malloc(sizeof(ASTNode));
        $$->type = NodeType_BinOp;
        $$->binOp.operator = '+';
        $$->binOp.left = $1;
        $$->binOp.right = $3;
    }
    | Expr MINUS Expr 
    {
        printf("PARSER: Recognized subtraction expression\n");
        $$ = malloc(sizeof(ASTNode));
        $$->type = NodeType_BinOp;
        $$->binOp.operator = '-';
        $$->binOp.left = $1;
        $$->binOp.right = $3;
    }
    | Expr MUL Expr 
    {
        printf("PARSER: Recognized multiplication expression\n");
        $$ = malloc(sizeof(ASTNode));
        $$->type = NodeType_BinOp;
        $$->binOp.operator = '*';
        $$->binOp.left = $1;
        $$->binOp.right = $3;
    }
    | Expr LOGICOP Expr 
    {
        printf("Parsed Logical Expression: %s %s %s\n", $1, $2, $3);
        $$ = createNode(NodeType_LogicalOp);
        $$->logicalOp.logicalOp = strdup($2);  // Store the operator string
        $$->logicalOp.left = $1;
        $$->logicalOp.right = $3;
    }
    | '(' Expr ')' 
    {
        printf("Parsed Expression in parentheses\n");
        $$ = $2;
    }
    | ID 
    {
        printf("Parsed Identifier: %s\n", $1);
        $$ = malloc(sizeof(ASTNode));
        $$->type = NodeType_SimpleID;
        $$->simpleID.name = strdup($1);
    } 
    | NUMBER 
    {
        printf("Parsed Number: %d\n", $1);
        $$ = malloc(sizeof(ASTNode));
        $$->type = NodeType_SimpleExpr;
        $$->simpleExpr.number = $1;
    }
    | ID '[' Expr ']'
    {
        printf("Parsed Array Access: %s[%s]\n", $1, $3);
        $$ = createNode(NodeType_ArrayAccess);
        $$->arrayAccess.arrayName = strdup($1);
        $$->arrayAccess.index = $3;
    }
    | TRUE
    {
        printf("Parsed TRUE bool");
        $$ = malloc(sizeof(ASTNode));
        $$->type = NodeType_SimpleExpr;
        $$->simpleExpr.number = 1;
    }
    | FALSE
    {
        printf("Parsed FALSE bool");
        $$ = malloc(sizeof(ASTNode));
        $$->type = NodeType_SimpleExpr;
        $$->simpleExpr.number = 0;
    }
    ;

%% 

void yyerror(const char *s) 
{
    printf("Error: %s\n", s);
    exit(1);
}

int main() 
{
    // Initialize the input source
    yyin = fopen("input.cmm", "r");

    // Initialize symbol table
    symTab = createSymbolTable(TABLE_SIZE);
    if (symTab == NULL) 
    {
        fprintf(stderr, "Error: Unable to initialize symbol table\n");
        exit(1);
    }

    initializeTempVars();

    if (yyparse() == 0) 
    {
        printf("=================Semantic=================\n");

        // Semantic Analysis
        semanticAnalysis(root, symTab);

        printTACToFile("TACsem.ir", tacHead);

        printf("=================Optimizer=================\n");
        // TAC Optimization
        optimizeTAC(&tacHead);  // 'tacHead' is the global head of the TAC linked list

        printTACToFile("TACopt.ir", tacHead);
        // Optionally print the optimized TAC to console
        // printCurrentOptimizedTAC(&tacHead);

        printf("=================Code Generation=================\n");

        // Code Generation
        initCodeGenerator("output.asm");
        generateMIPS(tacHead, symTab);  // Generate MIPS code from optimized TAC
        finalizeCodeGenerator("output.asm");
        printTACToFile("TACgen.ir", tacHead);
    }

    freeTACList(tacHead);

    // Traverse and free the AST
    if (root != NULL) 
    {
        printf("Starting to free AST\n");
        traverseAST(root, 0);
        freeAST(root);
    }

    freeSymbolTable(symTab);
    fclose(yyin);
    return 0;
}
