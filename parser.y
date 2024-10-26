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
SymbolTable* symTab = NULL;

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
%token <operator> ASSIGNOP PLUS MINUS MUL LOGICOP
%token <character> SEMICOLON '(' ')' '[' ']' '{' '}'
%token THEN DO TRUE FALSE

%left '+' '-'
%left '*' '/'
%nonassoc UMINUS

%left PLUS MINUS
%left MUL

%type <ast> Program VarDeclList VarDecl Stmt StmtList Expr Block ReturnStmt FuncDeclList FuncDecl MainFuncDecl ParamList //FuncCall argument_list arg

%start Program

%% 

Program:
    FuncDeclList
    {
        printf("Parsed Program\n");
        root = malloc(sizeof(ASTNode));
        root->type = NodeType_Program;
        root->program.funcDeclList = $1; // Function declarations list
    }
    ;

FuncDeclList:
    FuncDecl 
    { 
        $$ = $1; 
    }
    | FuncDeclList FuncDecl 
    { 
        $$ = createNode(NodeType_FuncDeclList); 
    }
    |
    {
        //Deleet this soon
    }
    ;

FuncDecl:
    MainFuncDecl  // Special case for main
    | TYPE ID '(' ParamList ')' '{' VarDeclList Block ReturnStmt '}'  
    {
        // Create function node with type, name, params, var decls, block
        printf("Parsed Function Declaration: %s\n", $2);
        $$ = createNode(NodeType_FuncDecl);
        $$->funcDecl.returnType = strdup($1);
        $$->funcDecl.funcName = strdup($2);
        $$->funcDecl.paramList = $4;
        $$->funcDecl.varDeclList = $7;
        $$->funcDecl.block = $8;
        $$->funcDecl.returnStmt = $9;
    }
    | VOID ID '(' ParamList ')' '{' VarDeclList Block '}'  
    {
        // Create function node with type, name, params, var decls, block
        printf("Parsed Function Declaration: %s\n", $2);
        $$ = createNode(NodeType_FuncDecl);
        $$->funcDecl.returnType = strdup("void");
        $$->funcDecl.funcName = strdup($2);
        $$->funcDecl.paramList = $4;
        $$->funcDecl.varDeclList = $7;
        $$->funcDecl.block = $8;
    }
    ;

MainFuncDecl:
    TYPE MAIN '(' ')' '{' VarDeclList Block '}' 
    {
        // Create function node for main
        printf("Parsed Main Function Declaration\n");
        $$ = createNode(NodeType_FuncDecl);
        $$->funcDecl.returnType = strdup($1);      // The return type of 'main'
        $$->funcDecl.funcName = strdup("main");    // Function name is "main"
        $$->funcDecl.paramList = NULL;             // Main has no parameters
        $$->funcDecl.varDeclList = $6;             // VarDeclList is at position 6
        $$->funcDecl.block = $7;                   // Block is at position 7
    }
    ;

ParamList:
    TYPE ID 
    {
        printf("Parsed single parameter: %s %s\n", $1, $2);
        $$ = createNode(NodeType_Param);
        $$->param.paramType = strdup($1); // Single parameter node
        $$->param.paramName = strdup($2);
    }
    | ParamList ',' TYPE ID 
    {
        printf("Parsed multiple parameters\n");
        ASTNode* newParamList = createNode(NodeType_ParamList);
        newParamList->paramList.param = createNode(NodeType_Param);
        newParamList->paramList.param->param.paramType = strdup($3);
        newParamList->paramList.param->param.paramName = strdup($4);
        newParamList->paramList.nextParam = NULL;

        // Link new parameter list node to existing list
        ASTNode* current = $1;
        while (current->paramList.nextParam) {
            current = current->paramList.nextParam;
        }
        current->paramList.nextParam = newParamList;
        $$ = $1;
    }
    | /* empty */ 
    {
        printf("Parsed empty parameter list\n");
        $$ = NULL; // No parameters
    }
    ;
    
// argument_list:
//     arg {
//         printf("Parsed single argument\n");
//         $$ = createArgumentNode($1); // Single argument expression
//     }
//     | argument_list ',' Expr {
//         printf("Parsed multiple arguments\n");
//         $$ = appendArgumentList($1, createArgumentNode($3)); // Append to argument list
//     }
//     | /* empty */ {
//         printf("Parsed empty argument list\n");
//         $$ = NULL; // No arguments
//     }
//     ; 
// 
// arg:
    
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

        if (strcmp($1, "float") == 0) {
            $$->varDecl.isFloat = true;  // Add a flag to indicate float
        } else {
            $$->varDecl.isFloat = false;
        }
    }
    | TYPE ID '[' NUMBER ']' SEMICOLON
    {
        $$ = createNode(NodeType_ArrayDecl);
        $$->arrayDecl.varType = strdup($1);
        $$->arrayDecl.varName = strdup($2);
        $$->arrayDecl.size = $4;

        if (strcmp($1, "float") == 0) {
            $$->arrayDecl.isFloat = true;  // Handle float arrays
        } else {
            $$->arrayDecl.isFloat = false;
        }
    }
    | TYPE ID '[' FLOAT_NUMBER ']' SEMICOLON
    {
        fatalError("Array index must be an integer, not a floating-point number.");
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

ReturnStmt:
    RETURN Expr SEMICOLON 
    {
        printf("Parsed Return Statement\n");
        $$ = createNode(NodeType_ReturnStmt);
        $$->returnStmt.expr = $2;
    }

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
        if ($3->type == NodeType_SimpleExpr && $3->simpleExpr.isFloat) 
        {
            fatalError("Array index must be an integer, not a floating-point number.");
        }
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
    | FLOAT_NUMBER
    {
        printf("Parsed Float Number: %f\n", $1);
        $$ = malloc(sizeof(ASTNode));
        $$->type = NodeType_SimpleExpr;
        $$->simpleExpr.floatValue = $1;
        $$->simpleExpr.isFloat = true;
    }
    | NUMBER 
    {
        printf("Parsed Number: %d\n", $1);
        $$ = malloc(sizeof(ASTNode));
        $$->type = NodeType_SimpleExpr;
        $$->simpleExpr.number = $1;
        $$->simpleExpr.isFloat = false;
    }
    | ID '[' Expr ']'
    {
        if ($3->type == NodeType_SimpleExpr && $3->simpleExpr.isFloat) 
        {
            fatalError("Array index must be an integer, not a floating-point number.");
        }
        printf("Parsed Array Access: %s[%s]\n", $1, $3);
        $$ = createNode(NodeType_ArrayAccess);
        $$->arrayAccess.arrayName = strdup($1);
        $$->arrayAccess.index = $3;
    }
    | TRUE
    {
        printf("Parsed TRUE bool\n");
        $$ = malloc(sizeof(ASTNode));
        $$->type = NodeType_SimpleExpr;
        $$->simpleExpr.number = 1;
    }
    | FALSE
    {
        printf("Parsed FALSE bool\n");
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

void fatalError(const char *s) 
{
    fprintf(stderr, "Fatal Error at line %d: %s\n", yylineno, s);
    exit(1);  // Exit the program with a non-zero status
}

int main() 
{
    // Initialize the input source
    yyin = fopen("input.cmm", "r");

    // Initialize symbol table
    symTab = createSymbolTable(TABLE_SIZE, NULL);
    if (symTab == NULL) 
    {
        fprintf(stderr, "Error: Unable to initialize symbol table\n");
        exit(1);
    }

    // Initialize temporary variables
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
