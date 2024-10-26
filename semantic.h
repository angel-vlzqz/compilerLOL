#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "AST.h"
#include "SymbolTable.h"
#include "Array.h"
#include "temp.h"

// Define a structure for TAC instructions

typedef struct TAC
{
    char *op;         // Operator
    char *arg1;       // Argument 1
    char *arg2;       // Argument 2
    char *result;     // Result
    struct TAC *next; // Next instruction
} TAC;

extern int tempVars[50];
extern TAC *tacHead; // Global head of the TAC instructions list

void semanticAnalysis(ASTNode *node, SymbolTable *symTab);
char *generateTACForExpr(ASTNode *expr, SymbolTable *symTab); // returns the TAC for the expression to print on console
char *createTempVar();
char *createOperand(ASTNode *node, SymbolTable *symTab);
void appendTAC(TAC **head, TAC *newInstruction);
void freeTACList(TAC *head);

#endif // SEMANTIC_H
