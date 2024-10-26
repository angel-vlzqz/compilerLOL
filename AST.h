#ifndef AST_H
#define AST_H

// Include standard libraries as needed, e.g., stdlib
// for memory management functions
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// NodeType enum to differentiate between different
// kinds of AST nodes
typedef enum
{
    NodeType_Program,
    NodeType_FuncDeclList,
    NodeType_FuncDecl,
    NodeType_ParamList,
    NodeType_Param,
    NodeType_VarDeclList,
    NodeType_VarDecl,
    NodeType_ArrayDecl,
    NodeType_StmtList,
    NodeType_AssignStmt,
    NodeType_ArrayAssign,
    NodeType_BinOp,
    NodeType_SimpleID,
    NodeType_SimpleExpr,
    NodeType_ArrayAccess,
    NodeType_Block,
    NodeType_ReturnStmt,
    NodeType_WriteStmt,
    NodeType_IfStmt,
    NodeType_WhileStmt,
    NodeType_LogicalOp,
    NodeType_Expr
} NodeType;

// Structure for AST nodes
typedef struct ASTNode
{
    NodeType type;
    char *dataType;
    union
    {
        struct
        {
            struct ASTNode *funcDeclList;
        } program;
        
        struct
        {
            struct ASTNode *funcDecl;
            struct ASTNode *nextFuncDecl; // Points to the next function in the list
        } funcDeclList;

        struct
        {
            char *returnType;            // Function return type, e.g., "int", "float"
            char *funcName;              // Function name
            struct ASTNode *paramList;   // Parameters of the function
            struct ASTNode *varDeclList; // Variable declarations within the function
            struct ASTNode *block;       // Function body (statements)
            struct ASTNode *returnStmt;
        } funcDecl;

        struct
        {
            char *paramType;            // Type of the parameter, e.g., "int", "float"
            char *paramName;            // Name of the parameter
        } param;

        struct
        {
            struct ASTNode *param;      // First parameter in the list
            struct ASTNode *nextParam;  // Next parameter in the list (recursively structured)
        } paramList;

        struct
        {
            struct ASTNode *varDecl;
            struct ASTNode *varDeclList;
        } varDeclList;

        struct
        {
            char *varType;
            char *varName;
            bool isFloat;
        } varDecl;

        struct
        {
            int number;
            float floatValue;
            bool isFloat;
        } simpleExpr;

        struct
        {
            char *name;
        } simpleID;

        struct
        {
            // Expression-specific fields
            char operator;         // Example for an operator field
            struct ASTNode *left;  // Left operand
            struct ASTNode *right; // Right operand
        } expr;

        struct
        {
            // StatementList-specific fields
            struct ASTNode *stmt;
            struct ASTNode *stmtList;
            // Example for linking statements in a list
        } stmtList;

        struct
        {
            char *operator; // e.g., '='
            char *varName;
            struct ASTNode *expr;
        } assignStmt;

        struct
        {
            char operator;
            struct ASTNode *left;
            struct ASTNode *right;
        } binOp;

        struct
        {
            char *logicalOp;
            struct ASTNode *left;
            struct ASTNode *right;
        } logicalOp; // Logical operation

        struct
        {
            struct ASTNode *stmtList;
        } block; // Block

        struct
        {
            struct ASTNode *expr; // Expression to return
        } returnStmt;

        struct
        {
            struct ASTNode *expr;
        } writeStmt; // WRITE statement

        struct
        {
            struct ASTNode *condition;
            struct ASTNode *thenBlock;
            struct ASTNode *elseBlock;
        } ifStmt; // IF-ELSE statement

        struct
        {
            struct ASTNode *condition;
            struct ASTNode *block;
        } whileStmt; // WHILE statement

        struct
        {
            char *varType;
            char *varName;
            int size;
            bool isFloat;
        } arrayDecl; // Array declaration

        struct
        {
            char *arrayName;
            struct ASTNode *index;
            struct ASTNode *expr;
        } arrayAssign; // Array assignment

        struct
        {
            char *arrayName;
            struct ASTNode *index;
        } arrayAccess; // Array access
    };
} ASTNode;

// Function prototypes for AST handling
ASTNode *createNode(NodeType type);
void freeAST(ASTNode *node);
void traverseAST(ASTNode *node, int level);

#endif // AST_H
