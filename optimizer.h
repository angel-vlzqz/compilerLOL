#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "semantic.h"
#include <stdbool.h>
#include <ctype.h>

// Function to optimize the TAC instructions
void optimizeTAC(TAC **head);

// Utility functions to check if a string is a constant or a variable
bool isConstant(const char *str);
bool isVariable(const char *str);
bool hasSideEffect(TAC *instr);

// Optimization functions that return the number of changes made
int constantFolding(TAC **head);
int constantPropagation(TAC **head);
int copyPropagation(TAC **head);
int deadCodeElimination(TAC **head);

// Functions to print the optimized TAC
void printOptimizedTAC(const char *filename, TAC *head);
void printCurrentOptimizedTAC(TAC *current);

#endif // OPTIMIZER_H
