#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "AST.h"         // Assuming this includes the definition of ASTNode
#include "SymbolTable.h" // Assuming this includes the definition of SymbolTable
#include "temp.h"

// Define a structure for TAC instructions

typedef struct TAC {
    char* op;   // Operator
    char* arg1; // Argument 1
    char* arg2; // Argument 2
    char* result; // Result
    struct TAC* next; // Next instruction
} TAC;

extern int tempVars[50];
extern TAC* tacHead; // Global head of the TAC instructions list

int allocateNextAvailableTempVar(int tempVars[]);
void semanticAnalysis(ASTNode* node, SymbolTable* symTab);
TAC *generateTACForExpr(ASTNode *expr, SymbolTable *symTab); //returns the TAC for the expression to print on console
char* createTempVar();
char *createOperand(ASTNode *node, SymbolTable *symTab);
void printTAC(TAC* tac);
void initializeTempVars();
void deallocateTempVar(int tempVars[], int index);
int allocateNextAvailableTempVar(int tempVars[]);
void appendTAC(TAC** head, TAC* newInstruction);
void printTACToFile(const char* filename, TAC* tac);
void freeTACList(TAC* head);
// You can add more function declarations related to semantic analysis here

#endif // SEMANTIC_H
