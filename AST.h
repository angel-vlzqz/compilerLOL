#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

struct SymbolTable;

// NodeType enum to differentiate between different kinds of AST nodes
typedef enum
{
    NodeType_Program,
    NodeType_DeclList,
    NodeType_FuncDeclList,
    NodeType_FuncDecl,
    NodeType_ParamList,
    NodeType_Param,
    NodeType_FuncCall,
    NodeType_ArgList,
    NodeType_Arg,
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
    NodeType_CastExpr,
    NodeType_RelOp,
    NodeType_NotOp
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
            struct ASTNode *declList;
        } program;

        struct
        {
            struct ASTNode *decl;
            struct ASTNode *next;
        } declList;

        struct
        {
            char *returnType;
            char *funcName;
            struct ASTNode *paramList;
            struct ASTNode *varDeclList;
            struct ASTNode *block;
            struct ASTNode *returnStmt;
            struct SymbolTable *prevSymTab;
        } funcDecl;

        struct
        {
            struct ASTNode *param;
            struct ASTNode *nextParam;
        } paramList;

        struct
        {
            char *paramType;
            char *paramName;
        } param;

        struct
        {
            char *funcName;
            struct ASTNode *argList;
        } funcCall;

        struct
        {
            struct ASTNode *arg;
            struct ASTNode *argList;
        } argList;

        struct
        {
            struct ASTNode *expr;
        } arg;

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
            struct ASTNode *initialValue; // For variable initialization
        } varDecl;

        struct
        {
            char *varType;
            char *varName;
            int size;
            bool isFloat;
        } arrayDecl;

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
            char operator;
            struct ASTNode *left;
            struct ASTNode *right;
        } binOp;

        struct
        {
            char *type; // Type to cast to
            struct ASTNode *expr;
        } castExpr;

        struct
        {
            char *logicalOp; // e.g., "&&", "||"
            struct ASTNode *left;
            struct ASTNode *right;
        } logicalOp;

        struct
        {
            struct ASTNode *stmt;
            struct ASTNode *stmtList;
        } stmtList;

        struct
        {
            char *operator;
            char *varName;
            struct ASTNode *expr;
        } assignStmt;

        struct
        {
            char *arrayName;
            struct ASTNode *index;
            struct ASTNode *expr;
        } arrayAssign;

        struct
        {
            char *arrayName;
            struct ASTNode *index;
        } arrayAccess;

        struct
        {
            struct ASTNode *stmtList;
        } block;

        struct
        {
            struct ASTNode *expr;
        } returnStmt;

        struct
        {
            struct ASTNode *expr;
        } writeStmt;

        struct
        {
            struct ASTNode *condition;
            struct ASTNode *thenBlock;
            struct ASTNode *elseBlock; // Can be NULL, Block node, or IfStmt node
        } ifStmt;

        struct
        {
            struct ASTNode *condition;
            struct ASTNode *block;
        } whileStmt;

        struct
        {
            char *operator; // e.g., "==", "<", ">"
            struct ASTNode *left;
            struct ASTNode *right;
        } relOp;

        struct
        {
            struct ASTNode *expr;
        } notOp;

    };
} ASTNode;

// Function prototypes for AST handling
ASTNode *createNode(NodeType type);
void freeAST(ASTNode *node);
void traverseAST(ASTNode *node, int level);

#endif // AST_H
