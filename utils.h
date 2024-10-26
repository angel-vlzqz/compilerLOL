// utils.h
#ifndef UTILS_H
#define UTILS_H

#include "optimizer.h"
#include "semantic.h"

// ---- parser.y helpers ----

void fatal(const char *s);  // , int yylineno

// ---- optimizer.c Helpers ----

bool isConstant(const char* str);
bool isVariable(const char* str);

// ---- semantic.c Helpers ----

void initializeTempVars(); // int tempVars[], int size
int allocateNextAvailableTempVar(int tempVars[], int size);
void deallocateTempVar(int tempVars[], int index);
void printTACToFile(const char* filename, TAC* tac);

#endif // UTILS_H