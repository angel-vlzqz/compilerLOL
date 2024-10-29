#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "AST.h"      // Include AST definition
#include "semantic.h" // Include TAC definition
#include "Array.h"
#include "SymbolTable.h"
#include "optimizer.h"
#include <stdbool.h>
#include <ctype.h>

#define MAX_REGISTER_MAP_SIZE 100 // Adjusted to accommodate more variables if needed

// Number of available registers, excluding reserved ones
#define NUM_AVAILABLE_REGISTERS 8

// Reserved registers
#define ADDRESS_CALC_REGISTER "$t9"
#define BASE_ADDRESS_REGISTER "$t8"

// Float Register Map Entry
typedef struct
{
    char *variable;
    char *regName;
} FloatRegisterMapEntry;

// Register Map Entry
typedef struct
{
    char *variable;
    char *regName;
} RegisterMapEntry;

// Initializes code generation, setting up any necessary structures
void initCodeGenerator(const char *outputFilename);

// Generates MIPS assembly code from the provided TAC
void generateMIPS(TAC *tacInstructions, SymbolTable *symTab);
void processTACList(TAC *tacList, SymbolTable *symTab);
void generateTACOperation(TAC *current, SymbolTable *symTab, const char *currentFunctionName);

// Finalizes code generation, closing files and cleaning up
void finalizeCodeGenerator(const char *outputFilename);

// Function declarations for register allocation
const char *allocateRegister();
void deallocateRegister(const char *regName);
void initializeRegisterMap();
void freeRegisterMap();
void setRegisterForVariable(const char *variable, const char *regName);
const char *getRegisterForVariable(const char *variable);
bool isVariableInRegisterMap(const char *variable);
void removeVariableFromRegisterMap(const char *variable);

// Function to compute offset for array accesses
char *computeOffset(const char *indexOperand, int elementSize);

void loadOperand(const char *operand, const char *registerName);

bool isTemporaryVariable(const char *operand);

// Function to check if a variable is used later
bool isVariableUsedLater(TAC *current, const char *variable);

// Functions for float register allocation
const char *allocateFloatRegister();
void deallocateFloatRegister(const char *regName);
const char *getFloatRegisterForVariable(const char *variable);
void setFloatRegisterForVariable(const char *variable, const char *regName);
void loadFloatOperand(const char *operand, const char *reg);
bool isVariableInFloatRegisterMap(const char *variable);
void removeVariableFromFloatRegisterMap(const char *variable);

#endif // CODE_GENERATOR_H
