%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AST.h"
#include "SymbolTable.h"
#include "semantic.h"
#include "optimizer.h"
#include "codeGenerator.h"
#include "utils.h"

#define TABLE_SIZE 101

extern int yylex();
extern FILE* yyin;
extern int yylineno;

void yyerror(const char *s);
void fatalError(const char *s);

ASTNode* root = NULL;
SymbolTable* globalSymTab = NULL;  // Global symbol table

%}

%union 
{
    int number;
    float float_number;
    char character;
    char* string;
    char* operator;
    struct ASTNode* ast;
}

%token <number> NUMBER       
%token <float_number> FLOAT_NUMBER
%token <string> IF ELSE WHILE RETURN WRITE ID TYPE MAIN VOID
%token <operator> ASSIGNOP EQ_OP NE_OP LE_OP GE_OP LT_OP GT_OP AND_OP OR_OP NOT_OP
%token <character> SEMICOLON '(' ')' '[' ']' '{' '}' ',' '+' '-' '*' '/'
%token THEN DO TRUE FALSE

%left OR_OP
%left AND_OP
%right NOT_OP
%nonassoc EQ_OP NE_OP
%nonassoc LT_OP GT_OP LE_OP GE_OP
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS

%type <ast> Program DeclList Decl VarDeclList VarDecl Stmt StmtList Expr Block ReturnStmt IfStmt FuncDecl MainFuncDecl ParamList Param FuncCall ArgList Arg ElsePart Condition FuncBody

%start Program

%%

Program:
    DeclList
    {
        printf("Parsed Program\n");
        root = createNode(NodeType_Program);
        root->program.declList = $1;
    }
    ;

DeclList:
    Decl
    {
        $$ = createNode(NodeType_DeclList);
        $$->declList.decl = $1;
        $$->declList.next = NULL;
    }
    | Decl DeclList 
    {
        $$ = createNode(NodeType_DeclList);
        $$->declList.decl = $1;
        $$->declList.next = $2;
    }
    ;

Decl:
    VarDecl
    {
        $$ = $1;
    }
    | FuncDecl
    {
        $$ = $1;
    }
    ;

FuncDecl:
    MainFuncDecl 
    {
        $$ = $1;
    } // Special case for main
    | TYPE ID '(' ParamList ')' FuncBody
    {
        printf("Parsed Function Declaration: %s\n", $2);

        // Removed symbol table manipulation during parsing

        // Create the function node and attach the body
        $$ = createNode(NodeType_FuncDecl);
        $$->funcDecl.returnType = strdup($1);
        $$->funcDecl.funcName = strdup($2);
        $$->funcDecl.paramList = $4;
        $$->funcDecl.varDeclList = $6->funcDecl.varDeclList;
        $$->funcDecl.block = $6->funcDecl.block;
        $$->funcDecl.returnStmt = $6->funcDecl.returnStmt;

        // Removed creation of currentSymTab during parsing
    }
    | VOID ID '(' ParamList ')' FuncBody 
    {
        printf("Parsed Function Declaration: %s\n", $2);

        // Removed symbol table manipulation during parsing

        // Create the function node and attach the body
        $$ = createNode(NodeType_FuncDecl);
        $$->funcDecl.returnType = strdup("void");
        $$->funcDecl.funcName = strdup($2);
        $$->funcDecl.paramList = $4;
        $$->funcDecl.varDeclList = $6->funcDecl.varDeclList;
        $$->funcDecl.block = $6->funcDecl.block;
        $$->funcDecl.returnStmt = $6->funcDecl.returnStmt;

        // Removed creation of currentSymTab during parsing
    }
    ;

MainFuncDecl:
    TYPE MAIN '(' ')' FuncBody
    {
        printf("Parsed Main Function Declaration\n");

        // Removed symbol table manipulation during parsing

        // Create the function node and attach the body
        $$ = createNode(NodeType_FuncDecl);
        $$->funcDecl.returnType = strdup($1);
        $$->funcDecl.funcName = strdup("main");
        $$->funcDecl.paramList = NULL;
        $$->funcDecl.varDeclList = $5->funcDecl.varDeclList;
        $$->funcDecl.block = $5->funcDecl.block;
        $$->funcDecl.returnStmt = $5->funcDecl.returnStmt;

        // Removed creation of currentSymTab during parsing
    }
    ;


FuncBody:
    '{' VarDeclList Block ReturnStmt '}'
    {
        // Create a temporary node to hold the body components
        $$ = createNode(NodeType_FuncDecl);
        $$->funcDecl.varDeclList = $2;
        $$->funcDecl.block = $3;
        $$->funcDecl.returnStmt = $4;
    }
    | '{' VarDeclList Block '}' 
    {
        // Create a temporary node to hold the body components
        $$ = createNode(NodeType_FuncDecl);
        $$->funcDecl.varDeclList = $2;
        $$->funcDecl.block = $3;
        $$->funcDecl.returnStmt = NULL;
    }
    ;

ParamList:
    Param
    {
        printf("Parsed parameter list\n");
        $$ = createNode(NodeType_ParamList);
        $$->paramList.param = $1;
        $$->paramList.nextParam = NULL;
    }
    | ParamList ',' Param
    {
        printf("Parsed parameter list\n");
        // Attach the new parameter to the end of the existing list
        ASTNode* temp = $1;
        while (temp->paramList.nextParam != NULL) {
            temp = temp->paramList.nextParam;
        }
        ASTNode* newParamList = createNode(NodeType_ParamList);
        newParamList->paramList.param = $3;
        newParamList->paramList.nextParam = NULL;
        temp->paramList.nextParam = newParamList;
        $$ = $1;
    }
    | /* empty */
    {
        printf("Parsed empty parameter list\n");
        $$ = NULL;
    }
    ;

Param:
    TYPE ID
    {
        printf("Parsed parameter: %s %s\n", $1, $2);
        $$ = createNode(NodeType_Param);
        $$->param.paramType = strdup($1);
        $$->param.paramName = strdup($2);
    }
    ;

VarDeclList:
    VarDecl VarDeclList 
    {
        $$ = createNode(NodeType_VarDeclList);
        $$->varDeclList.varDecl = $1;
        $$->varDeclList.varDeclList = $2;
    }
    | /* empty */
    {
        $$ = NULL;
    }
    ;

VarDecl:
    TYPE ID SEMICOLON
    {
        printf("Parsed variable declaration: %s %s\n", $1, $2);

        // Removed symbol table manipulation during parsing

        $$ = createNode(NodeType_VarDecl);
        $$->varDecl.varType = strdup($1);
        $$->varDecl.varName = strdup($2);
        $$->varDecl.initialValue = NULL;
    }
    | TYPE ID ASSIGNOP Expr SEMICOLON
    {
        printf("Parsed variable declaration with initialization: %s %s\n", $1, $2);

        // Removed symbol table manipulation during parsing

        $$ = createNode(NodeType_VarDecl);
        $$->varDecl.varType = strdup($1);
        $$->varDecl.varName = strdup($2);
        $$->varDecl.initialValue = $4;
    }
    | TYPE ID '[' NUMBER ']' SEMICOLON
    {
        printf("Parsed array declaration: %s %s[%d]\n", $1, $2, $4);

        // Removed symbol table manipulation during parsing

        $$ = createNode(NodeType_ArrayDecl);
        $$->arrayDecl.varType = strdup($1);
        $$->arrayDecl.varName = strdup($2);
        $$->arrayDecl.size = $4;
    }
    | TYPE ID '[' FLOAT_NUMBER ']' SEMICOLON
    {
        fatalError("Array size must be an integer, not a floating-point number.");
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
        $$ = createNode(NodeType_StmtList);
        $$->stmtList.stmt = $1;
        $$->stmtList.stmtList = $2;
    }
    | /* empty */ 
    {
        $$ = NULL;
    }
    ;

Stmt:
    ID ASSIGNOP Expr SEMICOLON 
    {
        printf("Parsed Assignment Statement: %s = ...\n", $1);

        // Create the assignment node
        $$ = createNode(NodeType_AssignStmt);
        $$->assignStmt.varName = strdup($1);
        $$->assignStmt.operator = strdup($2);
        $$->assignStmt.expr = $3;
    }
    | ID '[' Expr ']' ASSIGNOP Expr SEMICOLON
    {
        printf("Parsed Array Assignment: %s[...]=...\n", $1);

        // Create the array assignment node
        $$ = createNode(NodeType_ArrayAssign);
        $$->arrayAssign.arrayName = strdup($1);
        $$->arrayAssign.index = $3;
        $$->arrayAssign.expr = $6;
    }
    | FuncCall
    | WRITE Expr SEMICOLON 
    {
        printf("Parsed Write Statement\n");
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
    | IfStmt
    {

    }
    ;

IfStmt:
    IF '(' Condition ')' '{' Block '}' ElsePart
    {
        printf("Parsed If Statement with parentheses\n");
        $$ = createNode(NodeType_IfStmt);
        $$->ifStmt.condition = $3;
        $$->ifStmt.thenBlock = $6;
        $$->ifStmt.elseBlock = $8;
    }
    | IF  '(' Condition ')' '{' Block '}'
    {
        printf("Parsed If Statement without parentheses\n");
        $$ = createNode(NodeType_IfStmt);
        $$->ifStmt.condition = $3;
        $$->ifStmt.thenBlock = $6;
        $$->ifStmt.elseBlock = NULL;
    }
    ;

ElsePart:
    ELSE IF '(' Condition ')' '{' Block '}' ElsePart
    {
        printf("Parsed Else-If Statement with parentheses\n");
        ASTNode *elseIfNode = createNode(NodeType_IfStmt);
        elseIfNode->ifStmt.condition = $4;
        elseIfNode->ifStmt.thenBlock = $7;
        elseIfNode->ifStmt.elseBlock = $9;
        $$ = elseIfNode;
    }
    | ELSE IF '(' Condition ')' '{' Block '}'
    {
        printf("Parsed Else-If Statement without parentheses\n");
        ASTNode *elseIfNode = createNode(NodeType_IfStmt);
        elseIfNode->ifStmt.condition = $4;
        elseIfNode->ifStmt.thenBlock = $7;
        elseIfNode->ifStmt.elseBlock = NULL;
        $$ = elseIfNode;
    }
    | ELSE '{' Block '}'
    {
        printf("Parsed Else Statement\n");
        $$ = $3; // Else block
    }
    | /* empty */
    {
        $$ = NULL; // No else part
    }
    ;

Condition:
    Condition OR_OP Condition
    {
        printf("Parsed OR condition\n");
        $$ = createNode(NodeType_LogicalOp);
        $$->logicalOp.logicalOp = strdup($2); // "||"
        $$->logicalOp.left = $1;
        $$->logicalOp.right = $3;
    }
    | Condition AND_OP Condition
    {
        printf("Parsed AND condition\n");
        $$ = createNode(NodeType_LogicalOp);
        $$->logicalOp.logicalOp = strdup($2); // "&&"
        $$->logicalOp.left = $1;
        $$->logicalOp.right = $3;
    }
    | NOT_OP Condition
    {
        printf("Parsed NOT condition\n");
        $$ = createNode(NodeType_NotOp);
        $$->notOp.expr = $2;
    }
    | '(' Condition ')'
    {
        printf("Parsed condition in parentheses\n");
        $$ = $2;
    }
    | Expr EQ_OP Expr
    {
        printf("Parsed EQ_OP condition\n");
        $$ = createNode(NodeType_RelOp);
        $$->relOp.operator = strdup($2); // "=="
        $$->relOp.left = $1;
        $$->relOp.right = $3;
    }
    | Expr NE_OP Expr
    {
        printf("Parsed NE_OP condition\n");
        $$ = createNode(NodeType_RelOp);
        $$->relOp.operator = strdup($2); // "!="
        $$->relOp.left = $1;
        $$->relOp.right = $3;
    }
    | Expr LT_OP Expr
    {
        printf("Parsed LT_OP condition\n");
        $$ = createNode(NodeType_RelOp);
        $$->relOp.operator = strdup($2); // "<"
        $$->relOp.left = $1;
        $$->relOp.right = $3;
    }
    | Expr LE_OP Expr
    {
        printf("Parsed LE_OP condition\n");
        $$ = createNode(NodeType_RelOp);
        $$->relOp.operator = strdup($2); // "<="
        $$->relOp.left = $1;
        $$->relOp.right = $3;
    }
    | Expr GT_OP Expr
    {
        printf("Parsed GT_OP condition\n");
        $$ = createNode(NodeType_RelOp);
        $$->relOp.operator = strdup($2); // ">"
        $$->relOp.left = $1;
        $$->relOp.right = $3;
    }
    | Expr GE_OP Expr
    {
        printf("Parsed GE_OP condition\n");
        $$ = createNode(NodeType_RelOp);
        $$->relOp.operator = strdup($2); // ">="
        $$->relOp.left = $1;
        $$->relOp.right = $3;
    }
    | Expr
    {
        $$ = $1; // An expression can also be a condition
    }
    ;

ReturnStmt:
    RETURN Expr SEMICOLON 
    {
        printf("Parsed Return Statement\n");
        $$ = createNode(NodeType_ReturnStmt);
        $$->returnStmt.expr = $2;
    }
    ;

FuncCall:
    ID '(' ArgList ')'
    {
        printf("Parsed function call with arguments: %s(...)\n", $1);

        // Create the function call node
        $$ = createNode(NodeType_FuncCall);
        $$->funcCall.funcName = strdup($1);
        $$->funcCall.argList = $3;
    }
    | ID '(' ')'
    {
        printf("Parsed function call: %s()\n", $1);

        // Create the function call node
        $$ = createNode(NodeType_FuncCall);
        $$->funcCall.funcName = strdup($1);
        $$->funcCall.argList = NULL;
    }
    ;

ArgList:
    Arg
    {
        $$ = createNode(NodeType_ArgList);
        $$->argList.arg = $1;
        $$->argList.argList = NULL;
    }
    | ArgList ',' Arg
    {
        $$ = createNode(NodeType_ArgList);
        $$->argList.arg = $3;
        $$->argList.argList = $1;
    }
    ;

Arg:
    Expr
    {
        $$ = createNode(NodeType_Arg);
        $$->arg.expr = $1;
    }
    ;

Expr:
    Expr '+' Expr 
    {
        printf("Parsed addition expression\n");
        $$ = createNode(NodeType_BinOp);
        $$->binOp.operator = '+';
        $$->binOp.left = $1;
        $$->binOp.right = $3;
    }
    | Expr '-' Expr 
    {
        printf("Parsed subtraction expression\n");
        $$ = createNode(NodeType_BinOp);
        $$->binOp.operator = '-';
        $$->binOp.left = $1;
        $$->binOp.right = $3;
    }
    | Expr '*' Expr 
    {
        printf("Parsed multiplication expression\n");
        $$ = createNode(NodeType_BinOp);
        $$->binOp.operator = '*';
        $$->binOp.left = $1;
        $$->binOp.right = $3;
    }
    | Expr '/' Expr
    {
        printf("Parsed division expression\n");
        $$ = createNode(NodeType_BinOp);
        $$->binOp.operator = '/';
        $$->binOp.left = $1;
        $$->binOp.right = $3;
    }
    | FuncCall
    {
        printf("Parsed function call as an expression\n");
        $$ = $1; // FuncCall node
    }
    | '(' Expr ')'
    {
        printf("Parsed expression in parentheses\n");
        $$ = $2;
    }
    | '(' TYPE ')' Expr
    {
        printf("Parsed casting expression\n");
        $$ = createNode(NodeType_CastExpr);
        $$->castExpr.type = strdup($2);
        $$->castExpr.expr = $4;
    }
    | ID
    {
        printf("Parsed Identifier: %s\n", $1);

        // Create the identifier node
        $$ = createNode(NodeType_SimpleID);
        $$->simpleID.name = strdup($1);
    } 
    | FLOAT_NUMBER
    {
        printf("Parsed Float Number: %f\n", $1);
        $$ = createNode(NodeType_SimpleExpr);
        $$->simpleExpr.floatValue = $1;
        $$->simpleExpr.isFloat = true;
    }
    | NUMBER 
    {
        printf("Parsed Number: %d\n", $1);
        $$ = createNode(NodeType_SimpleExpr);
        $$->simpleExpr.number = $1;
        $$->simpleExpr.isFloat = false;
    }
    | ID '[' Expr ']'
    {
        printf("Parsed Array Access: %s[...]\n", $1);

        // Create the array access node
        $$ = createNode(NodeType_ArrayAccess);
        $$->arrayAccess.arrayName = strdup($1);
        $$->arrayAccess.index = $3;
    }
    | TRUE
    {
        printf("Parsed TRUE bool\n");
        $$ = createNode(NodeType_SimpleExpr);
        $$->simpleExpr.number = 1;
        $$->simpleExpr.isFloat = false;
    }
    | FALSE
    {
        printf("Parsed FALSE bool\n");
        $$ = createNode(NodeType_SimpleExpr);
        $$->simpleExpr.number = 0;
        $$->simpleExpr.isFloat = false;
    }
    ;

%%

void yyerror(const char *s) 
{
    fprintf(stderr, "Error at line %d: %s\n", yylineno, s);
    exit(1);
}

void fatalError(const char *s) 
{
    fprintf(stderr, "Fatal Error at line %d: %s\n", yylineno, s);
    exit(1);
}

int main() 
{
    // Initialize the input source
    yyin = fopen("input.cmm", "r");

    // Removed initialization of currentSymTab during parsing

    // Initialize temporary variables
    initializeTempVars();

    if (yyparse() == 0) 
    {
        printf("=================Semantic=================\n");

        // Initialize global symbol table after parsing
        globalSymTab = createSymbolTable(TABLE_SIZE, NULL);
        if (globalSymTab == NULL) 
        {
            fprintf(stderr, "Error: Unable to initialize symbol table\n");
            exit(1);
        }

        // Semantic Analysis
        semanticAnalysis(root, globalSymTab);

        printTACToFile("TACsem.ir", tacHead);

        printf("=================Optimizer=================\n");
        // TAC Optimization
        optimizeTAC(&tacHead);  // 'tacHead' is the global head of the TAC linked list

        printTACToFile("TACopt.ir", tacHead);

        printf("=================Code Generation=================\n");

        // Code Generation
        initCodeGenerator("output.asm");
        generateMIPS(tacHead, globalSymTab);  // Generate MIPS code from optimized TAC
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

    freeAllSymbolTables(globalSymTab);
    fclose(yyin);
    return 0;
}