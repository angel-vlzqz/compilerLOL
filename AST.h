#ifndef AST_H
#define AST_H

// Include standard libraries as needed, e.g., stdlib 
// for memory management functions
#include <stdlib.h>
#include <stdio.h>

// NodeType enum to differentiate between different 
// kinds of AST nodes
typedef enum {
    NodeType_Program,
    NodeType_VarDeclList,
    NodeType_VarDecl, 
    NodeType_StmtList,
    NodeType_AssignStmt,
    NodeType_BinOp,
    NodeType_SimpleID,
    NodeType_SimpleExpr,
    NodeType_Type,
    NodeType_Block,
    NodeType_WriteStmt,
    NodeType_IfStmt,
    NodeType_WhileStmt,
    NodeType_ReturnStmt,
    NodeType_LogicalOp,
} NodeType;

// Structure for AST nodes
typedef struct ASTNode {
    NodeType type;
    union {
        struct{
            struct ASTNode* varDeclList;
            struct ASTNode* stmtList;
        } program;

        struct {
            struct ASTNode* varDecl;
            struct ASTNode* varDeclList;
        } varDeclList;

        struct {
            char* varType;
            char* varName;
        } varDecl;

        struct {
            int number;
        } simpleExpr;

         struct {
            char* name;
        } simpleID;

        struct {
            // Expression-specific fields
            char operator;  // Example for an operator field
            struct ASTNode* left;  // Left operand
            struct ASTNode* right; // Right operand
        } expr;

        struct {
            // StatementList-specific fields
            struct ASTNode* stmt;
            struct ASTNode*  stmtList; 
            // Example for linking statements in a list
        } stmtList;

        struct {
            char* operator; // e.g., '='
            char* varName;
            struct ASTNode* expr;
        } assignStmt;

        struct {
            char operator;
            struct ASTNode* left;
            struct ASTNode* right;
        } binOp;

        struct {
            char* logicalOp; 
            struct ASTNode* left;
            struct ASTNode* right;
        } logicalOp;  // Logical operation

        struct {
            struct ASTNode* stmtList;
        } block;  // Block

        struct {
            char* varName;
        } writeStmt;  // WRITE statement

        struct {
            struct ASTNode* condition;
            struct ASTNode* thenBlock;
            struct ASTNode* elseBlock;
        } ifStmt;  // IF-ELSE statement

        struct {
            struct ASTNode* condition;
            struct ASTNode* block;
        } whileStmt;  // WHILE statement

        struct {
            struct ASTNode* expr;
        } returnStmt;  // RETURN statement
    };
} ASTNode;

// Function prototypes for AST handling
ASTNode* createNode(NodeType type);
void freeAST(ASTNode* node);
void traverseAST(ASTNode* node, int level);

#endif // AST_H
