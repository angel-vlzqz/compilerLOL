// codeGenerator.c
#include "codeGenerator.h"
#include "SymbolTable.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

// Output file pointer
static FILE *outputFile;

// Available integer registers
const char *availableRegisters[] = {"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7"};
#define NUM_AVAILABLE_REGISTERS 8
bool registerInUse[NUM_AVAILABLE_REGISTERS] = {false};

// Available floating-point registers (excluding $f0)
const char *availableFloatRegisters[] = {"$f2", "$f4", "$f6", "$f8", "$f10", "$f12", "$f14"};
#define NUM_AVAILABLE_FLOAT_REGISTERS 7
bool floatRegisterInUse[NUM_AVAILABLE_FLOAT_REGISTERS] = {false};

// Temporary registers for address calculations
#define ADDRESS_CALC_REGISTER "$t9"
#define BASE_ADDRESS_REGISTER "$t8"

// Register mapping sizes
#define MAX_REGISTER_MAP_SIZE 100
RegisterMapEntry registerMap[MAX_REGISTER_MAP_SIZE];

#define MAX_FLOAT_REGISTER_MAP_SIZE 100
FloatRegisterMapEntry floatRegisterMap[MAX_FLOAT_REGISTER_MAP_SIZE];

// Counters for function arguments and float constants
static int currentIntArgCount = 0;
static int currentFloatArgCount = 0;

// Buffer to store float constants
#define MAX_FLOAT_CONSTANTS 100
static char *floatConstants[MAX_FLOAT_CONSTANTS];
static int floatConstCount = 0;

// Track if the last function call returned a float
static bool lastCallReturnFloat = false;

// Utility function to check if an operand is a temporary variable (e.g., t0, t1)
bool isTemporaryVariable(const char *operand)
{
    if (operand == NULL)
        return false;
    if (operand[0] != 't')
        return false;
    for (int i = 1; operand[i] != '\0'; i++)
    {
        if (!isdigit((unsigned char)operand[i]))
            return false;
    }
    return true;
}

// Initialize the integer register map
void initializeRegisterMap()
{
    for (int i = 0; i < MAX_REGISTER_MAP_SIZE; i++)
    {
        registerMap[i].variable = NULL;
        registerMap[i].regName = NULL;
    }
}

// Initialize the floating-point register map
void initializeFloatRegisterMap()
{
    for (int i = 0; i < MAX_FLOAT_REGISTER_MAP_SIZE; i++)
    {
        floatRegisterMap[i].variable = NULL;
        floatRegisterMap[i].regName = NULL;
    }
}

// Free memory allocated for the integer register map
void freeRegisterMap()
{
    for (int i = 0; i < MAX_REGISTER_MAP_SIZE; i++)
    {
        if (registerMap[i].variable != NULL)
        {
            free(registerMap[i].variable);
            free(registerMap[i].regName);
            registerMap[i].variable = NULL;
            registerMap[i].regName = NULL;
        }
    }
}

// Free memory allocated for the floating-point register map
void freeFloatRegisterMap()
{
    for (int i = 0; i < MAX_FLOAT_REGISTER_MAP_SIZE; i++)
    {
        if (floatRegisterMap[i].variable != NULL)
        {
            free(floatRegisterMap[i].variable);
            free(floatRegisterMap[i].regName);
            floatRegisterMap[i].variable = NULL;
            floatRegisterMap[i].regName = NULL;
        }
    }
}

// Initialize the code generator by opening the output file and setting up register maps
void initCodeGenerator(const char *outputFilename)
{
    outputFile = fopen(outputFilename, "w");
    if (outputFile == NULL)
    {
        perror("Failed to open output file");
        exit(EXIT_FAILURE);
    }
    initializeRegisterMap();
    initializeFloatRegisterMap();
}

// Finalize the code generator by closing the output file and freeing register maps
void finalizeCodeGenerator(const char *outputFilename)
{
    if (outputFile)
    {
        fclose(outputFile);
        printf("MIPS code generated and saved to file %s\n", outputFilename);
        outputFile = NULL;
    }
    freeRegisterMap();
    freeFloatRegisterMap();
}

// Check if an operand is a floating-point constant
bool isFloatConstant(const char *operand)
{
    if (!operand)
        return false;
    return (isConstant(operand) && strchr(operand, '.') != NULL);
}

// Check if a variable is of type float
bool isFloatVariable(const char *variable, SymbolTable *symTab)
{
    if (!variable)
        return false;
    Symbol *sym = findSymbol(symTab, variable);
    if (sym && strcmp(sym->type, "float") == 0)
        return true;
    return false;
}

// Allocate an available integer register
const char *allocateRegister()
{
    for (int i = 0; i < NUM_AVAILABLE_REGISTERS; i++)
    {
        if (!registerInUse[i])
        {
            registerInUse[i] = true;
            return availableRegisters[i];
        }
    }

    fprintf(stderr, "Error: No available integer registers\n");
    exit(EXIT_FAILURE);
}

// Deallocate an integer register
void deallocateRegister(const char *regName)
{
    if (regName == NULL)
        return;
    for (int i = 0; i < NUM_AVAILABLE_REGISTERS; i++)
    {
        if (strcmp(regName, availableRegisters[i]) == 0)
        {
            registerInUse[i] = false;
            return;
        }
    }
}

// Map a variable to an integer register
void setRegisterForVariable(const char *variable, const char *regName)
{
    if (!variable || !regName) return;
    // Remove old entry if exists
    for (int i = 0; i < MAX_REGISTER_MAP_SIZE; i++)
    {
        if (registerMap[i].variable && strcmp(registerMap[i].variable, variable) == 0)
        {
            free(registerMap[i].variable);
            free(registerMap[i].regName);
            registerMap[i].variable = NULL;
            registerMap[i].regName = NULL;
        }
    }

    for (int i = 0; i < MAX_REGISTER_MAP_SIZE; i++)
    {
        if (registerMap[i].variable == NULL)
        {
            registerMap[i].variable = strdup(variable);
            if (!registerMap[i].variable)
            {
                perror("Failed to duplicate variable name");
                exit(EXIT_FAILURE);
            }
            registerMap[i].regName = strdup(regName);
            if (!registerMap[i].regName)
            {
                perror("Failed to duplicate register name");
                free(registerMap[i].variable);
                exit(EXIT_FAILURE);
            }
            return;
        }
    }
    fprintf(stderr, "Error: Register map is full\n");
    exit(EXIT_FAILURE);
}

// Retrieve the register mapped to a variable
const char *getRegisterForVariable(const char *variable)
{
    if (!variable) return NULL;
    for (int i = 0; i < MAX_REGISTER_MAP_SIZE; i++)
    {
        if (registerMap[i].variable != NULL && strcmp(registerMap[i].variable, variable) == 0)
        {
            return registerMap[i].regName;
        }
    }
    return NULL;
}

// Check if a variable is already mapped to an integer register
bool isVariableInRegisterMap(const char *variable)
{
    return getRegisterForVariable(variable) != NULL;
}

// Remove a variable from the integer register map
void removeVariableFromRegisterMap(const char *variable)
{
    for (int i = 0; i < MAX_REGISTER_MAP_SIZE; i++)
    {
        if (registerMap[i].variable != NULL && strcmp(registerMap[i].variable, variable) == 0)
        {
            free(registerMap[i].variable);
            free(registerMap[i].regName);
            registerMap[i].variable = NULL;
            registerMap[i].regName = NULL;
            break;
        }
    }
}

// Allocate an available floating-point register
const char *allocateFloatRegister()
{
    for (int i = 0; i < NUM_AVAILABLE_FLOAT_REGISTERS; i++)
    {
        if (!floatRegisterInUse[i])
        {
            floatRegisterInUse[i] = true;
            return availableFloatRegisters[i];
        }
    }
    fprintf(stderr, "Error: No available floating-point registers\n");
    exit(EXIT_FAILURE);
}

// Deallocate a floating-point register
void deallocateFloatRegister(const char *regName)
{
    if (!regName) return;
    for (int i = 0; i < NUM_AVAILABLE_FLOAT_REGISTERS; i++)
    {
        if (strcmp(availableFloatRegisters[i], regName) == 0)
        {
            floatRegisterInUse[i] = false;
            return;
        }
    }
}

// Map a variable to a floating-point register
void setFloatRegisterForVariable(const char *variable, const char *regName)
{
    if (!variable || !regName) return;
    // Remove old entry if exists
    for (int i = 0; i < MAX_FLOAT_REGISTER_MAP_SIZE; i++)
    {
        if (floatRegisterMap[i].variable && strcmp(floatRegisterMap[i].variable, variable) == 0)
        {
            free(floatRegisterMap[i].variable);
            free(floatRegisterMap[i].regName);
            floatRegisterMap[i].variable = NULL;
            floatRegisterMap[i].regName = NULL;
        }
    }

    for (int i = 0; i < MAX_FLOAT_REGISTER_MAP_SIZE; i++)
    {
        if (floatRegisterMap[i].variable == NULL)
        {
            floatRegisterMap[i].variable = strdup(variable);
            if (!floatRegisterMap[i].variable)
            {
                perror("Failed to duplicate variable name for float register");
                exit(EXIT_FAILURE);
            }
            floatRegisterMap[i].regName = strdup(regName);
            if (!floatRegisterMap[i].regName)
            {
                perror("Failed to duplicate float register name");
                free(floatRegisterMap[i].variable);
                exit(EXIT_FAILURE);
            }
            return;
        }
    }
    fprintf(stderr, "Error: Float register map is full\n");
    exit(EXIT_FAILURE);
}

// Retrieve the floating-point register mapped to a variable
const char *getFloatRegisterForVariable(const char *variable)
{
    if (!variable) return NULL;
    for (int i = 0; i < MAX_FLOAT_REGISTER_MAP_SIZE; i++)
    {
        if (floatRegisterMap[i].variable != NULL && strcmp(floatRegisterMap[i].variable, variable) == 0)
        {
            return floatRegisterMap[i].regName;
        }
    }
    return NULL;
}

// Check if a variable is already mapped to a floating-point register
bool isVariableInFloatRegisterMap(const char *variable)
{
    return getFloatRegisterForVariable(variable) != NULL;
}

// Remove a variable from the floating-point register map
void removeVariableFromFloatRegisterMap(const char *variable)
{
    for (int i = 0; i < MAX_FLOAT_REGISTER_MAP_SIZE; i++)
    {
        if (floatRegisterMap[i].variable != NULL && strcmp(floatRegisterMap[i].variable, variable) == 0)
        {
            free(floatRegisterMap[i].variable);
            free(floatRegisterMap[i].regName);
            floatRegisterMap[i].variable = NULL;
            floatRegisterMap[i].regName = NULL;
            break;
        }
    }
}

// Check if a variable is used later in the TAC list
bool isVariableUsedLater(TAC *current, const char *variable)
{
    TAC *temp = current->next;
    while (temp != NULL)
    {
        if ((temp->arg1 && strcmp(temp->arg1, variable) == 0) ||
            (temp->arg2 && strcmp(temp->arg2, variable) == 0) ||
            (temp->result && strcmp(temp->result, variable) == 0))
        {
            return true;
        }
        temp = temp->next;
    }
    return false;
}

// Add a floating-point constant to the floatConstants buffer and return its index
int addFloatConstantToData(const char *value)
{
    if (floatConstCount >= MAX_FLOAT_CONSTANTS)
    {
        fprintf(stderr, "Error: Too many float constants\n");
        exit(EXIT_FAILURE);
    }

    floatConstants[floatConstCount] = strdup(value);
    if (!floatConstants[floatConstCount])
    {
        perror("Failed to duplicate float constant");
        exit(EXIT_FAILURE);
    }

    return floatConstCount++;
}

// Load an integer constant into a register
void loadIntegerConstant(const char *reg, const char *value)
{
    if (strchr(value, '.'))
    {
        int intVal = (int)atof(value);
        fprintf(outputFile, "\tli %s, %d\n", reg, intVal);
    }
    else
    {
        int intVal = atoi(value);
        fprintf(outputFile, "\tli %s, %d\n", reg, intVal);
    }
}

// Handle the operand 'v0' based on the context (float or int)
static const char *handleV0Operand(bool floatContext)
{
    // If lastCallReturnFloat is true and floatContext is true, return $f0
    // If lastCallReturnFloat is false and not floatContext, return $v0
    // If mismatch occurs, handle by converting.
    if (lastCallReturnFloat && floatContext) {
        // Return floating-point register $f0
        return "$f0";
    } else if (!lastCallReturnFloat && !floatContext) {
        // Return integer register $v0
        return "$v0";
    } else if (lastCallReturnFloat && !floatContext) {
        // Float in $f0, convert to int
        const char *intReg = allocateRegister();
        fprintf(outputFile, "\tcvt.w.s $f0, $f0\n");
        fprintf(outputFile, "\tmfc1 %s, $f0\n", intReg);
        fprintf(outputFile, "# Converted float return value in $f0 to integer register %s\n", intReg);
        return intReg; // Treat as a temp register
    } else {
        // Int in $v0, but float context needed
        const char *fReg = allocateFloatRegister();
        fprintf(outputFile, "\tmtc1 $v0, %s\n", fReg);
        fprintf(outputFile, "\tcvt.s.w %s, %s\n", fReg, fReg);
        fprintf(outputFile, "# Converted integer return value in $v0 to float register %s\n", fReg);
        return fReg;
    }
}

// Ensure that a float operand is loaded into a floating-point register
const char *ensureFloatInRegister(const char *operand, SymbolTable *symTab)
{
    if (operand && strcmp(operand, "v0") == 0)
    {
        return handleV0Operand(true);
    }

    // Float register from variable?
    if (isVariableInFloatRegisterMap(operand))
    {
        return getFloatRegisterForVariable(operand);
    }

    if (isFloatConstant(operand))
    {
        const char *fReg = allocateFloatRegister();
        int constIndex = addFloatConstantToData(operand);
        const char *tempReg = allocateRegister();
        fprintf(outputFile, "\t# Loading float constant %s into register %s\n", operand, fReg);
        fprintf(outputFile, "\tla %s, float_%d\n", tempReg, constIndex); // Correct label usage
        fprintf(outputFile, "\tlwc1 %s, 0(%s)\n", fReg, tempReg);
        deallocateRegister(tempReg);
        // Do not set in map, it's a constant
        return fReg;
    }
    else if (isConstant(operand) && !isFloatConstant(operand))
    {
        // Convert integer constant to float
        const char *intReg = allocateRegister();
        loadIntegerConstant(intReg, operand);
        const char *fReg = allocateFloatRegister();
        fprintf(outputFile, "\t# Converting integer constant %s in %s to float register %s\n", operand, intReg, fReg);
        fprintf(outputFile, "\tmtc1 %s, %s\n", intReg, fReg);
        fprintf(outputFile, "\tcvt.s.w %s, %s\n", fReg, fReg);
        deallocateRegister(intReg);
        // No map entry for constants
        return fReg;
    }

    // If operand is in integer register map, convert to float
    if (isVariableInRegisterMap(operand))
    {
        const char *intReg = getRegisterForVariable(operand);
        const char *fReg = allocateFloatRegister();
        fprintf(outputFile, "\t# Converting integer variable %s in %s to float register %s\n", operand, intReg, fReg);
        fprintf(outputFile, "\tmtc1 %s, %s\n", intReg, fReg);
        fprintf(outputFile, "\tcvt.s.w %s, %s\n", fReg, fReg);
        return fReg;
    }

    // Load from memory if not in any register
    Symbol *sym = findSymbol(symTab, operand);
    if (!sym && isTemporaryVariable(operand))
    {
        // Temporary variable not in memory; should not happen
        fprintf(stderr, "Error: Attempting to load temp '%s' from memory.\n", operand);
        exit(1);
    }

    const char *fReg = allocateFloatRegister();
    const char *tempReg = allocateRegister();
    fprintf(outputFile, "\t# Loading float variable %s into register %s\n", operand, fReg);
    fprintf(outputFile, "\tla %s, %s\n", tempReg, operand);
    fprintf(outputFile, "\tlwc1 %s, 0(%s)\n", fReg, tempReg);
    deallocateRegister(tempReg);

    // Store in map for future use
    setFloatRegisterForVariable(operand, fReg);
    return fReg;
}

// Ensure that an integer operand is loaded into an integer register
const char *ensureIntInRegister(const char *operand, SymbolTable *symTab)
{
    if (operand && strcmp(operand, "v0") == 0)
    {
        return handleV0Operand(false);
    }

    if (isVariableInRegisterMap(operand))
    {
        return getRegisterForVariable(operand);
    }
    else if (isConstant(operand) && !isFloatConstant(operand))
    {
        const char *reg = allocateRegister();
        fprintf(outputFile, "\t# Loading integer constant %s into register %s\n", operand, reg);
        loadIntegerConstant(reg, operand);
        // No map for constants
        return reg;
    }
    else if (isFloatConstant(operand))
    {
        // Convert float constant to integer
        const char *fReg = ensureFloatInRegister(operand, symTab);
        const char *intReg = allocateRegister();
        fprintf(outputFile, "\tcvt.w.s %s, %s\n", fReg, fReg);
        fprintf(outputFile, "\t# Moving converted float from %s to integer register %s\n", fReg, intReg);
        fprintf(outputFile, "\tmfc1 %s, %s\n", intReg, fReg);
        deallocateFloatRegister(fReg);

        return intReg;
    }
    else if (isVariableInFloatRegisterMap(operand))
    {
        const char *fReg = getFloatRegisterForVariable(operand);
        fprintf(outputFile, "\tcvt.w.s %s, %s\n", fReg, fReg);
        const char *intReg = allocateRegister();
        fprintf(outputFile, "\t# Moving converted float from %s to integer register %s\n", fReg, intReg);
        fprintf(outputFile, "\tmfc1 %s, %s\n", intReg, fReg);
        // Not removing from float map since variable still known. If not used again, storeIfNotUsedLater will handle.
        return intReg;
    }
    else
    {
        Symbol *sym = findSymbol(symTab, operand);
        if (!sym && isTemporaryVariable(operand))
        {
            fprintf(stderr, "Error: Attempting to load temp '%s' from memory.\n", operand);
            exit(1);
        }

        const char *reg = allocateRegister();
        const char *tempReg = allocateRegister();
        fprintf(outputFile, "\t# Loading integer variable %s into register %s\n", operand, reg);
        fprintf(outputFile, "\tla %s, %s\n", tempReg, operand);
        fprintf(outputFile, "\tlw %s, 0(%s)\n", reg, tempReg);
        deallocateRegister(tempReg);

        // Store in register map if not a constant
        setRegisterForVariable(operand, reg);
        return reg;
    }
}

// Store a variable's value from register back to memory
void storeVariableToMemory(const char *var, SymbolTable *symTab)
{
    if (!var) return;
    Symbol *sym = findSymbol(symTab, var);

    const char *reg = getRegisterForVariable(var);
    if (reg)
    {
        const char *tempReg = allocateRegister();
        fprintf(outputFile, "\t# Storing integer variable %s from register %s to memory\n", var, reg);
        fprintf(outputFile, "\tla %s, %s\n", tempReg, var);
        fprintf(outputFile, "\tsw %s, 0(%s)\n", reg, tempReg);
        deallocateRegister(tempReg);
        deallocateRegister(reg);
        removeVariableFromRegisterMap(var);
    }

    const char *freg = getFloatRegisterForVariable(var);
    if (freg)
    {
        const char *tempReg = allocateRegister();
        fprintf(outputFile, "\t# Storing float variable %s from register %s to memory\n", var, freg);
        fprintf(outputFile, "\tla %s, %s\n", tempReg, var);
        fprintf(outputFile, "\tswc1 %s, 0(%s)\n", freg, tempReg);
        deallocateRegister(tempReg);
        deallocateFloatRegister(freg);
        removeVariableFromFloatRegisterMap(var);
    }
}

// Store a variable to memory if it's not used later in the TAC list
void storeIfNotUsedLater(const char *var, TAC *current, SymbolTable *symTab)
{
    if (!var) return;

    // If var is a constant or 'v0', no storage needed
    if (isConstant(var) || strcmp(var, "v0") == 0)
    {
        return; 
    }

    bool laterUse = isVariableUsedLater(current, var);
    if (!laterUse)
    {
        // If it's a real variable, store to memory
        Symbol *sym = findSymbol(symTab, var);
        if (sym && !sym->isFunction && !isTemporaryVariable(var))
        {
            storeVariableToMemory(var, symTab);
        }
        else
        {
            // Otherwise, just free the registers
            const char *r = getRegisterForVariable(var);
            if (r)
            {
                deallocateRegister(r);
                removeVariableFromRegisterMap(var);
            }
            const char *fr = getFloatRegisterForVariable(var);
            if (fr)
            {
                deallocateFloatRegister(fr);
                removeVariableFromFloatRegisterMap(var);
            }
        }
    }
}

// Handle function arguments by moving them into $a0-$a3 and $f12-$f15
void handleFunctionArguments(TAC *current, SymbolTable *symTab)
{
    // Determine if the argument is float or int based on symbol table
    Symbol *argSym = findSymbol(symTab, current->arg1);
    bool isFloat = argSym && strcmp(argSym->type, "float") == 0;

    if (isFloat)
    {
        const char *fReg = ensureFloatInRegister(current->arg1, symTab);
        const char *argReg;

        switch (currentFloatArgCount)
        {
        case 0:
            argReg = "$f12";
            break;
        case 1:
            argReg = "$f13";
            break;
        case 2:
            argReg = "$f14";
            break;
        case 3:
            argReg = "$f15";
            break;
        default:
            fprintf(stderr, "Warning: More than 4 float arguments not supported\n");
            return;
        }

        fprintf(outputFile, "\t# Moving float argument %s into %s\n", current->arg1, argReg);
        fprintf(outputFile, "\tmov.s %s, %s\n", argReg, fReg);
        // If srcReg was a constant or not used again, free it
        if (isFloatConstant(current->arg1))
            deallocateFloatRegister(fReg);
        else
            storeIfNotUsedLater(current->arg1, current, symTab);

        currentFloatArgCount++;
    }
    else
    {
        const char *srcReg = ensureIntInRegister(current->arg1, symTab);
        const char *argReg;

        switch (currentIntArgCount)
        {
        case 0:
            argReg = "$a0";
            break;
        case 1:
            argReg = "$a1";
            break;
        case 2:
            argReg = "$a2";
            break;
        case 3:
            argReg = "$a3";
            break;
        default:
            fprintf(stderr, "Warning: More than 4 integer arguments not supported\n");
            return;
        }

        fprintf(outputFile, "\t# Moving integer argument %s into %s\n", current->arg1, argReg);
        fprintf(outputFile, "\tmove %s, %s\n", argReg, srcReg);

        // If srcReg was a constant or not used again, free it
        if (isConstant(current->arg1))
            deallocateRegister(srcReg);
        else
            storeIfNotUsedLater(current->arg1, current, symTab);

        currentIntArgCount++;
    }
}

// Generate MIPS instructions for a given TAC operation with added comments
void generateTACOperation(TAC *current, SymbolTable *symTab, const char *currentFunctionName)
{
    if (!current || !current->op)
        return;

    // Print debug information
    printf("Debug: Processing operation '%s' with arg1='%s', arg2='%s', result='%s'\n",
           current->op ? current->op : "NULL",
           current->arg1 ? current->arg1 : "NULL",
           current->arg2 ? current->arg2 : "NULL",
           current->result ? current->result : "NULL");

    // Handle specific operations
    if (strcmp(current->op, "call") == 0)
    {
        Symbol *funcSym = findSymbol(symTab, current->arg1);
        bool isFloatReturnType = (funcSym && strcmp(funcSym->type, "float") == 0);
        lastCallReturnFloat = isFloatReturnType;

        fprintf(outputFile, "\t# Calling function %s\n", current->arg1);
        fprintf(outputFile, "\tjal %s\n", current->arg1);
        currentIntArgCount = 0;
        currentFloatArgCount = 0;
        return;
    }

    if (strcmp(current->op, "param") == 0)
    {
        handleFunctionArguments(current, symTab);
        return;
    }

    // Array Store: []= arr[index] = value
    if (strcmp(current->op, "[]=") == 0)
    {
        // arr[index] = value
        const char *arrayName = current->arg1;
        const char *indexOperand = current->arg2;
        const char *valueOperand = current->result;

        fprintf(outputFile, "\t# TAC: %s[%s] = %s\n", arrayName, indexOperand, valueOperand);
        fprintf(outputFile, "\tla %s, %s\n", BASE_ADDRESS_REGISTER, arrayName);

        const char *valReg = ensureIntInRegister(valueOperand, symTab);
        const char *idxReg = ensureIntInRegister(indexOperand, symTab);
        fprintf(outputFile, "\tsll %s, %s, 2\t# Calculating byte offset for index\n", ADDRESS_CALC_REGISTER, idxReg);
        fprintf(outputFile, "\tadd %s, %s, %s\t# Calculating address of %s[%s]\n", ADDRESS_CALC_REGISTER, BASE_ADDRESS_REGISTER, ADDRESS_CALC_REGISTER, arrayName, indexOperand);
        fprintf(outputFile, "\tsw %s, 0(%s)\t# Storing value into %s[%s]\n", valReg, ADDRESS_CALC_REGISTER, arrayName, indexOperand);

        // Free registers if not used later
        if (isConstant(valueOperand))
            deallocateRegister(valReg);
        else
            storeIfNotUsedLater(valueOperand, current, symTab);

        if (isConstant(indexOperand))
            deallocateRegister(idxReg);
        else
            storeIfNotUsedLater(indexOperand, current, symTab);

        storeIfNotUsedLater(arrayName, current, symTab);

        return;
    }

    // Array Load: =[] arr[index] -> var
    if (strcmp(current->op, "=[]") == 0)
    {
        // var = arr[index]
        const char *arrayName = current->arg1;
        const char *indexOperand = current->arg2;
        const char *resultVar = current->result;

        fprintf(outputFile, "\t# TAC: %s = %s[%s]\n", resultVar, arrayName, indexOperand);
        fprintf(outputFile, "\tla %s, %s\n", BASE_ADDRESS_REGISTER, arrayName);

        const char *idxReg = ensureIntInRegister(indexOperand, symTab);
        fprintf(outputFile, "\tsll %s, %s, 2\t# Calculating byte offset for index\n", ADDRESS_CALC_REGISTER, idxReg);
        fprintf(outputFile, "\tadd %s, %s, %s\t# Calculating address of %s[%s]\n", ADDRESS_CALC_REGISTER, BASE_ADDRESS_REGISTER, ADDRESS_CALC_REGISTER, arrayName, indexOperand);
        fprintf(outputFile, "\tlw %s, 0(%s)\t# Loading value from %s[%s] into %s\n", ADDRESS_CALC_REGISTER, ADDRESS_CALC_REGISTER, arrayName, indexOperand, ADDRESS_CALC_REGISTER);

        // Assign to result variable
        setRegisterForVariable(resultVar, ADDRESS_CALC_REGISTER);

        // Free index register if not used later
        if (isConstant(indexOperand))
            deallocateRegister(idxReg);
        else
            storeIfNotUsedLater(indexOperand, current, symTab);

        storeIfNotUsedLater(arrayName, current, symTab);

        return;
    }

    // Assignment: = var = value
    if (strcmp(current->op, "=") == 0)
    {
        bool destFloat = isFloatVariable(current->result, symTab);
        if (destFloat)
        {
            const char *fReg = ensureFloatInRegister(current->arg1, symTab);
            // If result var not in float map, set it
            if (!isVariableInFloatRegisterMap(current->result))
                setFloatRegisterForVariable(current->result, fReg);
            else
            {
                // If already has a float register, move
                const char *destFReg = getFloatRegisterForVariable(current->result);
                fprintf(outputFile, "\t# TAC: %s = %s\n", current->result, current->arg1);
                fprintf(outputFile, "\tmov.s %s, %s\t# Moving float value to %s\n", destFReg, fReg, current->result);
                // Free fReg if it was a constant
                if (isFloatConstant(current->arg1))
                    deallocateFloatRegister(fReg);
            }

            // **Store the float register back to memory**
            storeVariableToMemory(current->result, symTab);
        }
        else
        {
            const char *srcReg = ensureIntInRegister(current->arg1, symTab);
            if (!isVariableInRegisterMap(current->result))
            {
                setRegisterForVariable(current->result, srcReg);
                fprintf(outputFile, "\t# TAC: %s = %s\n", current->result, current->arg1);
                fprintf(outputFile, "\tmove %s, %s\t# Moving integer value to %s\n", getRegisterForVariable(current->result), srcReg, current->result);
            }
            else
            {
                const char *destReg = getRegisterForVariable(current->result);
                fprintf(outputFile, "\t# TAC: %s = %s\n", current->result, current->arg1);
                fprintf(outputFile, "\tmove %s, %s\t# Moving integer value to %s\n", destReg, srcReg, current->result);
                if (isConstant(current->arg1))
                    deallocateRegister(srcReg);
            }
        }

        storeIfNotUsedLater(current->arg1, current, symTab);
        storeIfNotUsedLater(current->result, current, symTab);
        return;
    }

    // Floating-point Division: fdiv
    if (strcmp(current->op, "fdiv") == 0)
    {
        fprintf(outputFile, "\t# TAC: %s = %s / %s\n", current->result, current->arg1, current->arg2);
        const char *reg1 = ensureFloatInRegister(current->arg1, symTab);
        const char *reg2 = ensureFloatInRegister(current->arg2, symTab);
        const char *resultReg = allocateFloatRegister();
        fprintf(outputFile, "\tdiv.s %s, %s, %s\t# Performing floating-point division\n", resultReg, reg1, reg2);
        // Set result variable
        setFloatRegisterForVariable(current->result, resultReg);

        // **Store the float register back to memory**
        storeVariableToMemory(current->result, symTab);

        storeIfNotUsedLater(current->arg1, current, symTab);
        storeIfNotUsedLater(current->arg2, current, symTab);
        storeIfNotUsedLater(current->result, current, symTab);
        return;
    }

    // Integer Subtraction: -
    if (strcmp(current->op, "-") == 0)
    {
        fprintf(outputFile, "\t# TAC: %s = %s - %s\n", current->result, current->arg1, current->arg2);
        const char *reg1 = ensureIntInRegister(current->arg1, symTab);
        const char *reg2 = ensureIntInRegister(current->arg2, symTab);
        const char *resReg = allocateRegister();
        fprintf(outputFile, "\tsub %s, %s, %s\t# Performing integer subtraction\n", resReg, reg1, reg2);
        setRegisterForVariable(current->result, resReg);

        storeIfNotUsedLater(current->arg1, current, symTab);
        storeIfNotUsedLater(current->arg2, current, symTab);
        storeIfNotUsedLater(current->result, current, symTab);
        return;
    }

    // Integer Addition: add
    if (strcmp(current->op, "+") == 0)
    {
        fprintf(outputFile, "\t# TAC: %s = %s + %s\n", current->result, current->arg1, current->arg2);
        const char *reg1 = ensureIntInRegister(current->arg1, symTab);
        const char *reg2 = ensureIntInRegister(current->arg2, symTab);
        const char *resReg = allocateRegister();
        fprintf(outputFile, "\tadd %s, %s, %s\t# Performing integer addition\n", resReg, reg1, reg2);
        setRegisterForVariable(current->result, resReg);

        storeIfNotUsedLater(current->arg1, current, symTab);
        storeIfNotUsedLater(current->arg2, current, symTab);
        storeIfNotUsedLater(current->result, current, symTab);
        return;
    }

    // Integer Multiplication: mul
    if (strcmp(current->op, "*") == 0)
    {
        fprintf(outputFile, "\t# TAC: %s = %s * %s\n", current->result, current->arg1, current->arg2);
        const char *reg1 = ensureIntInRegister(current->arg1, symTab);
        const char *reg2 = ensureIntInRegister(current->arg2, symTab);
        const char *resReg = allocateRegister();
        fprintf(outputFile, "\tmul %s, %s, %s\t# Performing integer multiplication\n", resReg, reg1, reg2);
        setRegisterForVariable(current->result, resReg);

        storeIfNotUsedLater(current->arg1, current, symTab);
        storeIfNotUsedLater(current->arg2, current, symTab);
        storeIfNotUsedLater(current->result, current, symTab);
        return;
    }

    // Integer Division: div
    if (strcmp(current->op, "/") == 0)
    {
        fprintf(outputFile, "\t# TAC: %s = %s / %s\n", current->result, current->arg1, current->arg2);
        const char *reg1 = ensureIntInRegister(current->arg1, symTab);
        const char *reg2 = ensureIntInRegister(current->arg2, symTab);
        const char *resReg = allocateRegister();
        fprintf(outputFile, "\tdiv %s, %s, %s\t# Performing integer division\n", resReg, reg1, reg2);
        setRegisterForVariable(current->result, resReg);

        storeIfNotUsedLater(current->arg1, current, symTab);
        storeIfNotUsedLater(current->arg2, current, symTab);
        storeIfNotUsedLater(current->result, current, symTab);
        return;
    }

    // Floating-point Addition: fadd
    if (strcmp(current->op, "fadd") == 0)
    {
        fprintf(outputFile, "\t# TAC: %s = %s + %s\n", current->result, current->arg1, current->arg2);
        const char *reg1 = ensureFloatInRegister(current->arg1, symTab);
        const char *reg2 = ensureFloatInRegister(current->arg2, symTab);
        const char *resultReg = allocateFloatRegister();
        fprintf(outputFile, "\tadd.s %s, %s, %s\t# Performing floating-point addition\n", resultReg, reg1, reg2);
        setFloatRegisterForVariable(current->result, resultReg);

        // **Store the float register back to memory**
        storeVariableToMemory(current->result, symTab);

        storeIfNotUsedLater(current->arg1, current, symTab);
        storeIfNotUsedLater(current->arg2, current, symTab);
        storeIfNotUsedLater(current->result, current, symTab);
        return;
    }

    // Floating-point Subtraction: fsub
    if (strcmp(current->op, "fsub") == 0)
    {
        fprintf(outputFile, "\t# TAC: %s = %s - %s\n", current->result, current->arg1, current->arg2);
        const char *reg1 = ensureFloatInRegister(current->arg1, symTab);
        const char *reg2 = ensureFloatInRegister(current->arg2, symTab);
        const char *resultReg = allocateFloatRegister();
        fprintf(outputFile, "\tsub.s %s, %s, %s\t# Performing floating-point subtraction\n", resultReg, reg1, reg2);
        setFloatRegisterForVariable(current->result, resultReg);

        // **Store the float register back to memory**
        storeVariableToMemory(current->result, symTab);

        storeIfNotUsedLater(current->arg1, current, symTab);
        storeIfNotUsedLater(current->arg2, current, symTab);
        storeIfNotUsedLater(current->result, current, symTab);
        return;
    }

    // Floating-point Multiplication: fmul
    if (strcmp(current->op, "fmul") == 0)
    {
        fprintf(outputFile, "\t# TAC: %s = %s * %s\n", current->result, current->arg1, current->arg2);
        const char *reg1 = ensureFloatInRegister(current->arg1, symTab);
        const char *reg2 = ensureFloatInRegister(current->arg2, symTab);
        const char *resultReg = allocateFloatRegister();
        fprintf(outputFile, "\tmul.s %s, %s, %s\t# Performing floating-point multiplication\n", resultReg, reg1, reg2);
        setFloatRegisterForVariable(current->result, resultReg);

        // **Store the float register back to memory**
        storeVariableToMemory(current->result, symTab);

        storeIfNotUsedLater(current->arg1, current, symTab);
        storeIfNotUsedLater(current->arg2, current, symTab);
        storeIfNotUsedLater(current->result, current, symTab);
        return;
    }

    // Logical AND: and
    if (strcmp(current->op, "and") == 0)
    {
        fprintf(outputFile, "\t# TAC: %s = %s AND %s\n", current->result, current->arg1, current->arg2);
        const char *reg1 = ensureIntInRegister(current->arg1, symTab);
        const char *reg2 = ensureIntInRegister(current->arg2, symTab);
        const char *resReg = allocateRegister();
        fprintf(outputFile, "\tand %s, %s, %s\t# Performing logical AND\n", resReg, reg1, reg2);
        setRegisterForVariable(current->result, resReg);

        storeIfNotUsedLater(current->arg1, current, symTab);
        storeIfNotUsedLater(current->arg2, current, symTab);
        storeIfNotUsedLater(current->result, current, symTab);
        return;
    }

    // Logical OR: or
    if (strcmp(current->op, "or") == 0)
    {
        fprintf(outputFile, "\t# TAC: %s = %s OR %s\n", current->result, current->arg1, current->arg2);
        const char *reg1 = ensureIntInRegister(current->arg1, symTab);
        const char *reg2 = ensureIntInRegister(current->arg2, symTab);
        const char *resReg = allocateRegister();
        fprintf(outputFile, "\tor %s, %s, %s\t# Performing logical OR\n", resReg, reg1, reg2);
        setRegisterForVariable(current->result, resReg);

        storeIfNotUsedLater(current->arg1, current, symTab);
        storeIfNotUsedLater(current->arg2, current, symTab);
        storeIfNotUsedLater(current->result, current, symTab);
        return;
    }

    // Logical NOT: !
    if (strcmp(current->op, "!") == 0)
    {
        fprintf(outputFile, "\t# TAC: %s = NOT %s\n", current->result, current->arg1);
        const char *reg1 = ensureIntInRegister(current->arg1, symTab);
        const char *resReg = allocateRegister();
        fprintf(outputFile, "\tnot %s, %s\t# Performing logical NOT\n", resReg, reg1);
        setRegisterForVariable(current->result, resReg);

        storeIfNotUsedLater(current->arg1, current, symTab);
        storeIfNotUsedLater(current->result, current, symTab);
        return;
    }

    // Comparison Operations: ==, !=, <, <=, >, >=
    if (strcmp(current->op, "==") == 0 ||
        strcmp(current->op, "!=") == 0 ||
        strcmp(current->op, "<") == 0 ||
        strcmp(current->op, "<=") == 0 ||
        strcmp(current->op, ">") == 0 ||
        strcmp(current->op, ">=") == 0)
    {
        bool isFloatComparison = false;
        Symbol *leftSym = findSymbol(symTab, current->arg1);
        Symbol *rightSym = findSymbol(symTab, current->arg2);
        if ((leftSym && strcmp(leftSym->type, "float") == 0) ||
            (rightSym && strcmp(rightSym->type, "float") == 0))
        {
            isFloatComparison = true;
        }

        if (isFloatComparison)
        {
            fprintf(outputFile, "\t# TAC: %s = %s %s %s\n", current->result, current->arg1, current->op, current->arg2);
            const char *reg1 = ensureFloatInRegister(current->arg1, symTab);
            const char *reg2 = ensureFloatInRegister(current->arg2, symTab);
            const char *resReg = allocateRegister();

            // Generate floating-point comparison
            if (strcmp(current->op, "==") == 0)
            {
                fprintf(outputFile, "\tc.eq.s %s, %s\t# Floating-point equality comparison\n", reg1, reg2);
            }
            else if (strcmp(current->op, "!=") == 0)
            {
                fprintf(outputFile, "\tc.ne.s %s, %s\t# Floating-point inequality comparison\n", reg1, reg2);
            }
            else if (strcmp(current->op, "<") == 0)
            {
                fprintf(outputFile, "\tc.lt.s %s, %s\t# Floating-point less-than comparison\n", reg1, reg2);
            }
            else if (strcmp(current->op, "<=") == 0)
            {
                fprintf(outputFile, "\tc.lt.s %s, %s\t# Floating-point less-than comparison\n", reg1, reg2);
                fprintf(outputFile, "\tc.le.s %s, %s\t# Floating-point less-than-or-equal comparison\n", reg1, reg2);
            }
            else if (strcmp(current->op, ">") == 0)
            {
                fprintf(outputFile, "\tc.gt.s %s, %s\t# Floating-point greater-than comparison\n", reg1, reg2);
            }
            else if (strcmp(current->op, ">=") == 0)
            {
                fprintf(outputFile, "\tc.ge.s %s, %s\t# Floating-point greater-than-or-equal comparison\n", reg1, reg2);
            }

            // Set condition flag and assign boolean result
            fprintf(outputFile, "\tbc1t label_true_%s\t# Branch to label_true_%s if condition is true\n", current->result, current->result);
            fprintf(outputFile, "\tli %s, 0\t# Set result to 0 (false)\n", resReg);
            fprintf(outputFile, "\tj label_end_%s\t# Jump to label_end_%s\n", current->result, current->result);
            fprintf(outputFile, "label_true_%s:\n", current->result);
            fprintf(outputFile, "\tli %s, 1\t# Set result to 1 (true)\n", resReg);
            fprintf(outputFile, "label_end_%s:\n", current->result);

            setRegisterForVariable(current->result, resReg);

            storeIfNotUsedLater(current->arg1, current, symTab);
            storeIfNotUsedLater(current->arg2, current, symTab);
            storeIfNotUsedLater(current->result, current, symTab);
        }
        else
        {
            fprintf(outputFile, "\t# TAC: %s = %s %s %s\n", current->result, current->arg1, current->op, current->arg2);
            const char *reg1 = ensureIntInRegister(current->arg1, symTab);
            const char *reg2 = ensureIntInRegister(current->arg2, symTab);
            const char *resReg = allocateRegister();

            // Generate integer comparison
            if (strcmp(current->op, "==") == 0)
            {
                fprintf(outputFile, "\tseq %s, %s, %s\t# Integer equality comparison\n", resReg, reg1, reg2);
            }
            else if (strcmp(current->op, "!=") == 0)
            {
                fprintf(outputFile, "\tsne %s, %s, %s\t# Integer inequality comparison\n", resReg, reg1, reg2);
            }
            else if (strcmp(current->op, "<") == 0)
            {
                fprintf(outputFile, "\tslt %s, %s, %s\t# Integer less-than comparison\n", resReg, reg1, reg2);
            }
            else if (strcmp(current->op, "<=") == 0)
            {
                fprintf(outputFile, "\tsle %s, %s, %s\t# Integer less-than-or-equal comparison\n", resReg, reg1, reg2);
            }
            else if (strcmp(current->op, ">") == 0)
            {
                fprintf(outputFile, "\tsgt %s, %s, %s\t# Integer greater-than comparison\n", resReg, reg1, reg2);
            }
            else if (strcmp(current->op, ">=") == 0)
            {
                fprintf(outputFile, "\tsge %s, %s, %s\t# Integer greater-than-or-equal comparison\n", resReg, reg1, reg2);
            }

            setRegisterForVariable(current->result, resReg);

            storeIfNotUsedLater(current->arg1, current, symTab);
            storeIfNotUsedLater(current->arg2, current, symTab);
            storeIfNotUsedLater(current->result, current, symTab);
        }
        return;
    }

    // Goto: goto label
    if (strcmp(current->op, "goto") == 0)
    {
        fprintf(outputFile, "\t# TAC: goto %s\n", current->arg1);
        fprintf(outputFile, "\tj %s\t# Jumping to label %s\n", current->arg1, current->arg1);
        return;
    }

    // Label: label:
    if (strcmp(current->op, "label") == 0)
    {
        fprintf(outputFile, "\t# TAC: label %s\n", current->arg1);
        fprintf(outputFile, "%s:\t# Label %s\n", current->arg1, current->arg1);
        return;
    }

    // Write Operation: write, write_float
    if (strcmp(current->op, "write") == 0 || strcmp(current->op, "write_float") == 0)
    {
        if (strcmp(current->op, "write_float") == 0)
        {
            fprintf(outputFile, "\t# TAC: write_float %s\n", current->arg1);
            const char *reg = ensureFloatInRegister(current->arg1, symTab);
            fprintf(outputFile, "\tmov.s $f12, %s\t# Moving float to $f12 for syscall\n", reg);
            fprintf(outputFile, "\tli $v0, 2\t# Setting syscall for float output\n");
            fprintf(outputFile, "\tsyscall\t# Performing syscall to print float\n");
            deallocateFloatRegister(reg);
        }
        else
        {
            fprintf(outputFile, "\t# TAC: write %s\n", current->arg1);
            const char *reg = ensureIntInRegister(current->arg1, symTab);
            fprintf(outputFile, "\tmove $a0, %s\t# Moving integer to $a0 for syscall\n", reg);
            fprintf(outputFile, "\tli $v0, 1\t# Setting syscall for integer output\n");
            fprintf(outputFile, "\tsyscall\t# Performing syscall to print integer\n");
            deallocateRegister(reg);
        }
        return;
    }

    // Return Operation: return
    if (strcmp(current->op, "return") == 0)
    {
        if (current->arg1)
        {
            Symbol *retSym = findSymbol(symTab, currentFunctionName);
            if (retSym && strcmp(retSym->type, "float") == 0)
            {
                fprintf(outputFile, "\t# TAC: return %s\n", current->arg1);
                const char *fReg = ensureFloatInRegister(current->arg1, symTab);
                fprintf(outputFile, "\tmov.s $f0, %s\t# Moving float return value to $f0\n", fReg);
                deallocateFloatRegister(fReg);
            }
            else
            {
                fprintf(outputFile, "\t# TAC: return %s\n", current->arg1);
                const char *reg = ensureIntInRegister(current->arg1, symTab);
                fprintf(outputFile, "\tmove $v0, %s\t# Moving integer return value to $v0\n", reg);
                deallocateRegister(reg);
            }
        }
        fprintf(outputFile, "\t# Performing function return\n");
        fprintf(outputFile, "\tjr $ra\t# Jumping back to caller\n");
        return;
    }

    // Unsupported operation
    fprintf(stderr, "Warning: Unsupported TAC operation '%s'\n", current->op);
}

// Debug function to print the current state of the symbol table
void debugPrintSymbolTableState(SymbolTable *symTab, const char *context)
{
    printf("\nDebug: Symbol Table State at %s\n", context);
    printf("----------------------------------------\n");
    for (int i = 0; i < symTab->size; i++)
    {
        Symbol *current = symTab->table[i];
        while (current != NULL)
        {
            printf("Symbol: %s, Type: %s, IsArray: %s\n",
                   current->name,
                   current->type,
                   current->isArray ? "true" : "false");
            current = current->next;
        }
    }
    printf("----------------------------------------\n");
}

// Function to process the TAC list and generate corresponding MIPS code
void processTACList(TAC *tacList, SymbolTable *symTab)
{
    debugPrintSymbolTableState(symTab, "Start of TAC Processing");
    TAC *current = tacList;
    const char *currentFunctionName = NULL;
    bool inFunction = false;

    while (current != NULL)
    {
        printf("Processing TAC instruction: op=%s, arg1=%s, arg2=%s, result=%s\n",
               current->op ? current->op : "NULL",
               current->arg1 ? current->arg1 : "NULL",
               current->arg2 ? current->arg2 : "NULL",
               current->result ? current->result : "NULL");

        if (current->op != NULL)
        {
            if (strcmp(current->op, "label") == 0)
            {
                currentFunctionName = current->arg1;
                fprintf(outputFile, "%s:\t# Label for function %s\n", currentFunctionName, currentFunctionName);

                if (strcmp(currentFunctionName, "main") == 0)
                {
                    fprintf(outputFile, "\t.globl main\t# Declare main as global\n");
                }
                inFunction = true;
            }
            else if (strcmp(current->op, "prologue") == 0)
            {
                fprintf(outputFile, "\t# Prologue for function %s\n", currentFunctionName);
                fprintf(outputFile, "\taddiu $sp, $sp, -8\t# Allocate stack space\n");
                fprintf(outputFile, "\tsw $fp, 4($sp)\t# Save frame pointer\n");
                fprintf(outputFile, "\tsw $ra, 0($sp)\t# Save return address\n");
                fprintf(outputFile, "\tmove $fp, $sp\t# Set frame pointer\n");

                initializeRegisterMap();
                initializeFloatRegisterMap();
                memset(registerInUse, 0, sizeof(registerInUse));
                memset(floatRegisterInUse, 0, sizeof(floatRegisterInUse));

                currentIntArgCount = 0;
                currentFloatArgCount = 0;
            }
            else if (strcmp(current->op, "epilogue") == 0)
            {
                fprintf(outputFile, "\t# Epilogue for function %s\n", currentFunctionName);
                fprintf(outputFile, "%s_epilogue:\t# Epilogue label\n", currentFunctionName);

                fprintf(outputFile, "\tmove $sp, $fp\t# Restore stack pointer\n");
                fprintf(outputFile, "\tlw $fp, 4($sp)\t# Restore frame pointer\n");
                fprintf(outputFile, "\tlw $ra, 0($sp)\t# Restore return address\n");
                fprintf(outputFile, "\taddiu $sp, $sp, 8\t# Deallocate stack space\n");

                if (strcmp(currentFunctionName, "main") == 0)
                {
                    fprintf(outputFile, "\tli $v0, 10\t# Syscall to exit\n");
                    fprintf(outputFile, "\tsyscall\t# Exit program\n");
                }
                else
                {
                    // For other functions, simply return
                    fprintf(outputFile, "\tjr $ra\t# Return to caller\n");
                }

                inFunction = false;
            }
            else
            {
                if (!inFunction)
                {
                    current = current->next;
                    continue;
                }

                // Generate MIPS code for the current TAC operation with comments
                generateTACOperation(current, symTab, currentFunctionName);
            }
        }

        current = current->next;
    }
}

// Function to generate MIPS code from TAC instructions
void generateMIPS(TAC *tacInstructions, SymbolTable *symTab)
{
    // Collect all symbols from the symbol table
    int symbolCapacity = 100;
    int symbolCount = 0;
    Symbol **symbolList = malloc(symbolCapacity * sizeof(Symbol *));
    if (!symbolList)
    {
        fprintf(stderr, "Error: Memory allocation failed for symbol list.\n");
        exit(1);
    }
    collectAllSymbols(symTab, &symbolList, &symbolCount, &symbolCapacity);

    // Begin data segment
    fprintf(outputFile, ".data\n");

    // Variables already declared to avoid duplicates
    int declaredCapacity = 100;
    int declaredCount = 0;
    char **declaredVariables = malloc(declaredCapacity * sizeof(char *));
    if (declaredVariables == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for declared variables.\n");
        exit(EXIT_FAILURE);
    }

    // Declare variables in the data segment
    for (int i = 0; i < symbolCount; i++)
    {
        Symbol *symbol = symbolList[i];
        if (symbol->isFunction || isTemporaryVariable(symbol->name))
        {
            continue; // Skip functions and temporary variables
        }

        // Check if variable is already declared
        bool alreadyDeclared = false;
        for (int j = 0; j < declaredCount; j++)
        {
            if (strcmp(declaredVariables[j], symbol->name) == 0)
            {
                alreadyDeclared = true;
                break;
            }
        }
        if (alreadyDeclared)
        {
            continue;
        }

        // Resize declaredVariables array if needed
        if (declaredCount >= declaredCapacity)
        {
            declaredCapacity *= 2;
            declaredVariables = realloc(declaredVariables, declaredCapacity * sizeof(char *));
            if (!declaredVariables)
            {
                fprintf(stderr, "Error: Memory reallocation failed for declared variables.\n");
                exit(EXIT_FAILURE);
            }
        }

        declaredVariables[declaredCount++] = strdup(symbol->name);

        // Declare the variable based on its type
        if (symbol->isArray)
        {
            int totalSize = symbol->arrayInfo->size * 4; // Assuming 4 bytes per element
            fprintf(outputFile, "\t# Declaring array %s with size %d\n", symbol->name, totalSize);
            fprintf(outputFile, "%s: .space %d\n", symbol->name, totalSize);
        }
        else if (strcmp(symbol->type, "float") == 0)
        {
            if (symbol->value != NULL)
                fprintf(outputFile, "\t# Declaring float variable %s with initial value %s\n", symbol->name, symbol->value);
            else
                fprintf(outputFile, "\t# Declaring float variable %s with initial value 0.0\n", symbol->name);
            if (symbol->value != NULL)
                fprintf(outputFile, "%s: .float %s\n", symbol->name, symbol->value);
            else
                fprintf(outputFile, "%s: .float 0.0\n", symbol->name);
        }
        else if (strcmp(symbol->type, "int") == 0)
        {
            if (symbol->value != NULL)
                fprintf(outputFile, "\t# Declaring integer variable %s with initial value %s\n", symbol->name, symbol->value);
            else
                fprintf(outputFile, "\t# Declaring integer variable %s with initial value 0\n", symbol->name);
            if (symbol->value != NULL)
                fprintf(outputFile, "%s: .word %s\n", symbol->name, symbol->value);
            else
                fprintf(outputFile, "%s: .word 0\n", symbol->name);
        }
        else if (strcmp(symbol->type, "bool") == 0)
        {
            if (symbol->value != NULL)
            {
                int boolValue = (strcmp(symbol->value, "true") == 0) ? 1 : 0;
                fprintf(outputFile, "\t# Declaring boolean variable %s with initial value %d\n", symbol->name, boolValue);
                fprintf(outputFile, "%s: .word %d\n", symbol->name, boolValue);
            }
            else
            {
                fprintf(outputFile, "\t# Declaring boolean variable %s with initial value 0 (false)\n", symbol->name);
                fprintf(outputFile, "%s: .word 0\n", symbol->name);
            }
        }
    }

    // Declare float constants collected so far
    for (int i = 0; i < floatConstCount; i++)
    {
        fprintf(outputFile, "\t# Declaring float constant float_%d\n", i);
        fprintf(outputFile, "float_%d: .float %s\n", i, floatConstants[i]);
        free(floatConstants[i]); // Free after declaring
    }

    free(declaredVariables);
    free(symbolList);

    // Now begin text segment
    fprintf(outputFile, ".text\n");

    // Process the TAC instructions to generate MIPS code
    processTACList(tacInstructions, symTab);
}
