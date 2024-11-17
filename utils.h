// utils.h
#ifndef UTILS_H
#define UTILS_H

#include "optimizer.h"
#include "semantic.h"
#include "AST.h"

// Array of node type names for easy printing
extern const char *nodeTypeNames[];

// ---- parser.y helpers ----

void fatal(const char *s); // , int yylineno

// ---- optimizer.c Helpers ----

bool isConstant(const char *str);
bool isVariable(const char *str);

// ---- semantic.c Helpers ----

void initializeTempVars(); // int tempVars[], int size
int allocateNextAvailableTempVar(int tempVars[], int size);
void deallocateTempVar(int tempVars[], int index);
void printTACToFile(const char *filename, TAC *tac);
void printNodeDetails(ASTNode *node);

#endif // UTILS_H