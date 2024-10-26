// codeGenerator.h

#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "AST.h"      // Include AST definition
#include "semantic.h" // Include TAC definition
#include "Array.h"
#include "SymbolTable.h"
#include "optimizer.h"
#include <stdbool.h>
#include <ctype.h>

#define MAX_REGISTER_MAP_SIZE 32 // Adjusted to accommodate more variables if needed

// Number of available registers, excluding reserved ones
#define NUM_AVAILABLE_REGISTERS 9

// Reserved registers
#define ADDRESS_CALC_REGISTER "$t9"
#define BASE_ADDRESS_REGISTER "$t8"

typedef struct VarNode
{
    char *name;
    int initialValue;
    bool isInitialized;
    struct VarNode *next;
} VarNode;

// Structure for register mapping
typedef struct
{
    char *variable; // Variable name
    char *regName;  // Register name
} RegisterMapEntry;

// Initializes code generation, setting up any necessary structures
void initCodeGenerator(const char *outputFilename);

// Generates MIPS assembly code from the provided TAC
void generateMIPS(TAC *tacInstructions, SymbolTable *symTab);

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

// Print the current TAC instruction
void printCurrentTAC(TAC *tac);

VarNode *findVariable(VarNode *varList, const char *varName);

void loadOperand(const char *operand, const char *registerName);

void freeVariableList(VarNode *varList);

void collectVariables(TAC *tacInstructions, VarNode **varList);

bool isTemporaryVariable(const char *operand);

// Function to check if a variable is used later
bool isVariableUsedLater(TAC *current, const char *variable);

// Functions for float register allocation
const char *allocateFloatRegister();
void deallocateFloatRegister(const char *regName);

// helper function
char *computeOffset(const char *indexOperand, int elementSize);

#endif // CODE_GENERATOR_H
