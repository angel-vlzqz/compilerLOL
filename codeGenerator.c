#include "codeGenerator.h"
#include "SymbolTable.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

static FILE *outputFile;

// Available registers for int (excluding $t8 and $t9)
const char *availableRegisters[] = {"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7"};
#define NUM_AVAILABLE_REGISTERS 8
bool registerInUse[NUM_AVAILABLE_REGISTERS] = {false};

// Available registers for float (even-numbered $f registers)
const char *availableFloatRegisters[] = {"$f0", "$f2", "$f4", "$f6", "$f8", "$f10", "$f12", "$f14"};
#define NUM_AVAILABLE_FLOAT_REGISTERS 8
bool floatRegisterInUse[NUM_AVAILABLE_FLOAT_REGISTERS] = {false};

// Reserved registers
#define ADDRESS_CALC_REGISTER "$t9"
#define BASE_ADDRESS_REGISTER "$t8"

// Register map to keep track of variable to register mappings
#define MAX_REGISTER_MAP_SIZE 100
RegisterMapEntry registerMap[MAX_REGISTER_MAP_SIZE];

// Float register map
#define MAX_FLOAT_REGISTER_MAP_SIZE 100
FloatRegisterMapEntry floatRegisterMap[MAX_FLOAT_REGISTER_MAP_SIZE];

void initializeRegisterMap()
{
    for (int i = 0; i < MAX_REGISTER_MAP_SIZE; i++)
    {
        registerMap[i].variable = NULL;
        registerMap[i].regName = NULL;
    }
}

void initializeFloatRegisterMap()
{
    for (int i = 0; i < MAX_FLOAT_REGISTER_MAP_SIZE; i++)
    {
        floatRegisterMap[i].variable = NULL;
        floatRegisterMap[i].regName = NULL;
    }
}

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

void initCodeGenerator(const char *outputFilename)
{
    outputFile = fopen(outputFilename, "w");
    if (outputFile == NULL)
    {
        perror("Failed to open output file");
        exit(EXIT_FAILURE);
    }
    initializeRegisterMap();
    initializeFloatRegisterMap(); // Initialize float register map
}

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

/* Register Allocation Functions */

// Allocate a register
const char *allocateRegister()
{
    for (int i = 0; i < NUM_AVAILABLE_REGISTERS; i++)
    {
        if (!registerInUse[i])
        {
            registerInUse[i] = true;
            return availableRegisters[i]; // Return the register name
        }
    }

    // Implement spilling logic here
    fprintf(stderr, "Error: No available integer registers\n");
    exit(1);
}

// Deallocate a register
void deallocateRegister(const char *regName)
{
    for (int i = 0; i < NUM_AVAILABLE_REGISTERS; i++)
    {
        if (strcmp(regName, availableRegisters[i]) == 0)
        {
            registerInUse[i] = false;
            break;
        }
    }
}

// Set register for variable in the register map
void setRegisterForVariable(const char *variable, const char *regName)
{
    for (int i = 0; i < MAX_REGISTER_MAP_SIZE; i++)
    {
        if (registerMap[i].variable == NULL)
        {
            registerMap[i].variable = strdup(variable);
            registerMap[i].regName = strdup(regName);
            break;
        }
    }
}

// Get register assigned to a variable
const char *getRegisterForVariable(const char *variable)
{
    for (int i = 0; i < MAX_REGISTER_MAP_SIZE; i++)
    {
        if (registerMap[i].variable != NULL && strcmp(registerMap[i].variable, variable) == 0)
        {
            return registerMap[i].regName;
        }
    }
    return NULL; // Variable not found in the map
}

// Check if variable is in the register map
bool isVariableInRegisterMap(const char *variable)
{
    return getRegisterForVariable(variable) != NULL;
}

// Remove variable from register map
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

/* Float Register Allocation Functions */

// Allocate a float register
const char *allocateFloatRegister()
{
    for (int i = 0; i < NUM_AVAILABLE_FLOAT_REGISTERS; i++)
    {
        if (!floatRegisterInUse[i])
        {
            floatRegisterInUse[i] = true;
            return availableFloatRegisters[i]; // Return the register name
        }
    }
    fprintf(stderr, "Error: No available floating-point registers\n");
    exit(1);
}

// Deallocate a float register
void deallocateFloatRegister(const char *regName)
{
    for (int i = 0; i < NUM_AVAILABLE_FLOAT_REGISTERS; i++)
    {
        if (strcmp(availableFloatRegisters[i], regName) == 0)
        {
            floatRegisterInUse[i] = false;
            return;
        }
    }
}

// Get the float register allocated to a variable
const char *getFloatRegisterForVariable(const char *variable)
{
    for (int i = 0; i < MAX_FLOAT_REGISTER_MAP_SIZE; i++)
    {
        if (floatRegisterMap[i].variable != NULL && strcmp(floatRegisterMap[i].variable, variable) == 0)
        {
            return floatRegisterMap[i].regName;
        }
    }
    return NULL;
}

// Map a variable to a float register
void setFloatRegisterForVariable(const char *variable, const char *regName)
{
    for (int i = 0; i < MAX_FLOAT_REGISTER_MAP_SIZE; i++)
    {
        if (floatRegisterMap[i].variable == NULL)
        {
            floatRegisterMap[i].variable = strdup(variable);
            floatRegisterMap[i].regName = strdup(regName);
            return;
        }
    }
    fprintf(stderr, "Error: Float register map is full\n");
    exit(1);
}

// Check if variable is in the float register map
bool isVariableInFloatRegisterMap(const char *variable)
{
    return getFloatRegisterForVariable(variable) != NULL;
}

// Remove variable from float register map
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

/* Other Helper Functions */

// Function to compute offset for array access if index is a constant
char *computeOffset(const char *indexOperand, int elementSize)
{
    if (isConstant(indexOperand))
    {
        int indexValue = atoi(indexOperand);
        int offset = indexValue * elementSize;
        char *offsetStr = (char *)malloc(16);
        snprintf(offsetStr, 16, "%d", offset);
        return offsetStr;
    }
    return NULL; // Index is not constant
}

void loadOperand(const char *operand, const char *registerName)
{
    if (isConstant(operand))
    {
        if (registerName[0] == '$' && registerName[1] == 'f')
        {
            // Load immediate float constant into a float register
            fprintf(outputFile, "\tli.s %s, %s\n", registerName, operand);
        }
        else
        {
            // Load integer constant
            fprintf(outputFile, "\tli %s, %s\n", registerName, operand);
        }
    }
    else if (isVariableInRegisterMap(operand))
    {
        // Operand is in an integer register
        const char *reg = getRegisterForVariable(operand);
        if (strcmp(registerName, reg) != 0)
        {
            fprintf(outputFile, "\tmove %s, %s\n", registerName, reg);
        }
    }
    else if (isVariableInFloatRegisterMap(operand))
    {
        // Operand is in a float register
        const char *reg = getFloatRegisterForVariable(operand);
        if (strcmp(registerName, reg) != 0)
        {
            fprintf(outputFile, "\tmov.s %s, %s\n", registerName, reg);
        }
    }
    else if (isTemporaryVariable(operand))
    {
        fprintf(stderr, "Error: Temporary variable %s not in register map\n", operand);
        exit(1);
    }
    else
    {
        if (registerName[0] == '$' && registerName[1] == 'f')
        {
            // Load float from memory
            fprintf(outputFile, "\tl.s %s, %s\n", registerName, operand);
        }
        else
        {
            // Load integer from memory
            fprintf(outputFile, "\tlw %s, %s\n", registerName, operand);
        }
    }
}

void loadFloatOperand(const char *operand, const char *reg)
{
    if (isConstant(operand))
    {
        // Load immediate float constant into a float register
        fprintf(outputFile, "\tli.s %s, %s\n", reg, operand);
    }
    else if (isVariableInFloatRegisterMap(operand))
    {
        // Operand is in a float register
        const char *srcReg = getFloatRegisterForVariable(operand);
        if (strcmp(reg, srcReg) != 0)
        {
            fprintf(outputFile, "\tmov.s %s, %s\n", reg, srcReg);
        }
    }
    else
    {
        // Load from memory
        fprintf(outputFile, "\tl.s %s, %s\n", reg, operand);
    }
}

bool isTemporaryVariable(const char *operand)
{
    if (operand == NULL)
        return false;
    // Check if operand starts with 't' followed by digits
    if (operand[0] != 't')
        return false;
    for (int i = 1; operand[i] != '\0'; i++)
    {
        if (!isdigit(operand[i]))
        {
            return false;
        }
    }
    return true;
}

// Function to check if a variable is used later
bool isVariableUsedLater(TAC *current, const char *variable)
{
    TAC *temp = current->next;
    while (temp != NULL)
    {
        if ((temp->arg1 != NULL && strcmp(temp->arg1, variable) == 0) ||
            (temp->arg2 != NULL && strcmp(temp->arg2, variable) == 0) ||
            (temp->result != NULL && strcmp(temp->result, variable) == 0))
        {
            return true;
        }
        temp = temp->next;
    }
    return false;
}

void generateMIPS(TAC *tacInstructions, SymbolTable *symTab)
{
    // Collect all symbols from symbol tables
    int symbolCapacity = 100;
    int symbolCount = 0;
    Symbol **symbolList = malloc(symbolCapacity * sizeof(Symbol *));
    collectAllSymbols(symTab, &symbolList, &symbolCount, &symbolCapacity);

    // Generate .data section
    fprintf(outputFile, ".data\n");

    // Declare variables from the collected symbols
    // Keep track of declared variable names to avoid duplicates
    int declaredCapacity = 100;
    int declaredCount = 0;
    char **declaredVariables = malloc(declaredCapacity * sizeof(char *));
    if (declaredVariables == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for declared variables.\n");
        exit(1);
    }

    for (int i = 0; i < symbolCount; i++)
    {
        Symbol *symbol = symbolList[i];
        if (symbol->isFunction || isTemporaryVariable(symbol->name))
        {
            continue;
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

        // Add variable name to declaredVariables
        if (declaredCount >= declaredCapacity)
        {
            declaredCapacity *= 2;
            declaredVariables = realloc(declaredVariables, declaredCapacity * sizeof(char *));
            if (declaredVariables == NULL)
            {
                fprintf(stderr, "Error: Memory allocation failed while expanding declared variables.\n");
                exit(1);
            }
        }
        declaredVariables[declaredCount++] = strdup(symbol->name);

        // Now declare the variable
        if (symbol->isArray)
        {
            int totalSize = symbol->arrayInfo->size * 4; // Assuming 4 bytes per element
            fprintf(outputFile, "%s: .space %d\n", symbol->name, totalSize);
        }
        else if (strcmp(symbol->type, "float") == 0)
        {
            // Check if the float symbol has a value
            if (symbol->value != NULL)
            {
                // Print the float value in the .data section
                fprintf(outputFile, "%s: .float %s\n", symbol->name, symbol->value);
            }
            else
            {
                // If no value is set, default to 0.0
                fprintf(outputFile, "%s: .float 0.0\n", symbol->name);
            }
        }
        else if (strcmp(symbol->type, "bool") == 0)
        {
            // Initialize boolean variables to 0 (false)
            fprintf(outputFile, "%s: .word 0\n", symbol->name);
        }
        else
        {
            // For temporary variables and other types
            fprintf(outputFile, "%s: .word 0\n", symbol->name);
        }
    }

    // Free the declaredVariables array
    for (int i = 0; i < declaredCount; i++)
    {
        free(declaredVariables[i]);
    }
    free(declaredVariables);

    // Free the symbolList after use
    free(symbolList);

    // Start the .text section
    fprintf(outputFile, ".text\n");

    // Initialize pointers for processing TAC instructions
    TAC *current = tacInstructions;
    TAC *mainTAC = NULL;
    TAC *mainTACTail = NULL;
    TAC *otherFunctionsTAC = NULL;
    TAC *otherFunctionsTACTail = NULL;
    const char *currentFunctionName = NULL;
    bool inMainFunction = false;
    bool inFunction = false;

    // Split TAC instructions between main and other functions
    while (current != NULL)
    {
        if (current->op && strcmp(current->op, "label") == 0)
        {
            currentFunctionName = current->arg1;
            inFunction = true;

            if (strcmp(currentFunctionName, "main") == 0)
            {
                inMainFunction = true;
            }
            else
            {
                inMainFunction = false;
            }
        }

        // Add current instruction to appropriate list
        if (inFunction)
        {
            if (inMainFunction)
            {
                // Add to mainTAC list
                if (mainTAC == NULL)
                {
                    mainTAC = mainTACTail = current;
                }
                else
                {
                    mainTACTail->next = current;
                    mainTACTail = current;
                }
            }
            else
            {
                // Add to otherFunctionsTAC list
                if (otherFunctionsTAC == NULL)
                {
                    otherFunctionsTAC = otherFunctionsTACTail = current;
                }
                else
                {
                    otherFunctionsTACTail->next = current;
                    otherFunctionsTACTail = current;
                }
            }
        }

        current = current->next;
    }

    // Ensure that the tails of the lists point to NULL
    if (mainTACTail != NULL)
    {
        mainTACTail->next = NULL;
    }
    if (otherFunctionsTACTail != NULL)
    {
        otherFunctionsTACTail->next = NULL;
    }

    // Process main function first
    processTACList(mainTAC, symTab);

    // Add a newline between functions
    fprintf(outputFile, "\n");

    // Process other functions
    processTACList(otherFunctionsTAC, symTab);

    // Free symbolList after use
    // free(symbolList);

    // Clean up TAC lists if necessary
    // (Assuming you have a function to free TAC lists)
    // freeTACList(mainTAC);
    // freeTACList(otherFunctionsTAC);
}

void processTACList(TAC *tacList, SymbolTable *symTab)
{
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
                fprintf(outputFile, "%s:\n", currentFunctionName);

                if (strcmp(currentFunctionName, "main") == 0)
                {
                    fprintf(outputFile, ".globl main\n");
                }

                // Set inFunction to true when entering a function
                inFunction = true;
            }
            else if (strcmp(current->op, "prologue") == 0)
            {
                // Function prologue
                fprintf(outputFile, "# Prologue for function %s\n", currentFunctionName);

                // Adjust $sp and save $ra and $fp
                fprintf(outputFile, "\taddiu $sp, $sp, -8\n"); // Allocate stack space
                fprintf(outputFile, "\tsw $fp, 4($sp)\n");     // Save old frame pointer
                fprintf(outputFile, "\tsw $ra, 0($sp)\n");     // Save return address
                fprintf(outputFile, "\tmove $fp, $sp\n");      // Set frame pointer

                // Initialize register maps and register usage for the function
                initializeRegisterMap();
                initializeFloatRegisterMap();
                memset(registerInUse, 0, sizeof(registerInUse));
                memset(floatRegisterInUse, 0, sizeof(floatRegisterInUse));
            }
            else if (strcmp(current->op, "epilogue") == 0)
            {
                // Function epilogue
                fprintf(outputFile, "# Epilogue for function %s\n", currentFunctionName);
                fprintf(outputFile, "%s_epilogue:\n", currentFunctionName);

                // Restore $fp and $ra
                fprintf(outputFile, "\tmove $sp, $fp\n");     // Restore stack pointer
                fprintf(outputFile, "\tlw $fp, 4($sp)\n");    // Restore old frame pointer
                fprintf(outputFile, "\tlw $ra, 0($sp)\n");    // Restore return address
                fprintf(outputFile, "\taddiu $sp, $sp, 8\n"); // Deallocate stack space

                // Exit the program if main function
                if (strcmp(currentFunctionName, "main") == 0)
                {
                    fprintf(outputFile, "\tli $v0, 10\n"); // Syscall code for exit
                    fprintf(outputFile, "\tsyscall\n");
                }
                else
                {
                    // Return to caller
                    fprintf(outputFile, "\tjr $ra\n");
                }

                // Set inFunction to false when exiting a function
                inFunction = false;
            }
            else
            {
                if (!inFunction)
                {
                    current = current->next;
                    continue;
                }

                // Call generateTACOperation
                generateTACOperation(current, symTab, currentFunctionName);
            }
        }

        current = current->next;
    }
}

void generateTACOperation(TAC *current, SymbolTable *symTab, const char *currentFunctionName)
{
    if (current->op == NULL)
    {
        return;
    }

    if (strcmp(current->op, "return") == 0)
    {
        // Return from function
        fprintf(outputFile, "# Return from function %s\n", currentFunctionName);
        if (current->arg1 != NULL)
        {
            // Move return value into $v0 or $f0
            if (isConstant(current->arg1))
            {
                fprintf(outputFile, "\tli $v0, %s\n", current->arg1);
            }
            else
            {
                const char *reg = getRegisterForVariable(current->arg1);
                if (!reg)
                {
                    reg = allocateRegister();
                    setRegisterForVariable(current->arg1, reg);
                    loadOperand(current->arg1, reg);
                }
                fprintf(outputFile, "\tmove $v0, %s\n", reg);
            }
        }
        // Jump to epilogue
        fprintf(outputFile, "\tj %s_epilogue\n", currentFunctionName);
    }
    else if (strcmp(current->op, "call") == 0)
    {
        // Function call
        fprintf(outputFile, "# Function call to %s\n", current->arg1);

        // Save caller-saved registers if necessary
        // For simplicity, not saving any in this example

        // Handle passing arguments if any
        // For simplicity, assuming no arguments

        // Call the function
        fprintf(outputFile, "\tjal %s\n", current->arg1);

        // Handle return value
        if (current->result != NULL)
        {
            // Map result variable to $v0 or $f0
            const char *resultReg = allocateRegister();
            setRegisterForVariable(current->result, resultReg);
            fprintf(outputFile, "\tmove %s, $v0\n", resultReg);
        }
    }
    else if (strcmp(current->op, "=") == 0)
    {
        // Assignment operation
        fprintf(outputFile, "# Generating MIPS code for assignment\n");

        const char *srcReg = getRegisterForVariable(current->arg1);
        if (!srcReg)
        {
            srcReg = allocateRegister();
            setRegisterForVariable(current->arg1, srcReg);
            loadOperand(current->arg1, srcReg);
        }

        const char *destReg = getRegisterForVariable(current->result);
        if (!destReg)
        {
            destReg = allocateRegister();
            setRegisterForVariable(current->result, destReg);
        }

        fprintf(outputFile, "\tmove %s, %s\n", destReg, srcReg);
    }
    else if (strcmp(current->op, "+") == 0 || strcmp(current->op, "-") == 0 ||
             strcmp(current->op, "*") == 0 || strcmp(current->op, "/") == 0)
    {
        // Generate code for integer binary operations
        fprintf(outputFile, "# Generating MIPS code for operation %s\n", current->op);
        // Load operands
        const char *reg1 = getRegisterForVariable(current->arg1);
        if (!reg1)
        {
            reg1 = allocateRegister();
            setRegisterForVariable(current->arg1, reg1);
            loadOperand(current->arg1, reg1);
        }
        const char *reg2 = getRegisterForVariable(current->arg2);
        if (!reg2)
        {
            reg2 = allocateRegister();
            setRegisterForVariable(current->arg2, reg2);
            loadOperand(current->arg2, reg2);
        }
        const char *resultReg = allocateRegister();
        setRegisterForVariable(current->result, resultReg);
        // Perform operation
        if (strcmp(current->op, "+") == 0)
        {
            fprintf(outputFile, "\tadd %s, %s, %s\n", resultReg, reg1, reg2);
        }
        else if (strcmp(current->op, "-") == 0)
        {
            fprintf(outputFile, "\tsub %s, %s, %s\n", resultReg, reg1, reg2);
        }
        else if (strcmp(current->op, "*") == 0)
        {
            fprintf(outputFile, "\tmul %s, %s, %s\n", resultReg, reg1, reg2);
        }
        else if (strcmp(current->op, "/") == 0)
        {
            fprintf(outputFile, "\tdiv %s, %s\n", reg1, reg2);
            fprintf(outputFile, "\tmflo %s\n", resultReg);
        }
    }
    else if (strcmp(current->op, "write") == 0)
    {
        // Write operation
        fprintf(outputFile, "# Generating MIPS code for write operation\n");
        const char *srcReg = getRegisterForVariable(current->arg1);
        if (!srcReg)
        {
            srcReg = allocateRegister();
            setRegisterForVariable(current->arg1, srcReg);
            loadOperand(current->arg1, srcReg);
        }
        // Move value to $a0
        fprintf(outputFile, "\tmove $a0, %s\n", srcReg);
        fprintf(outputFile, "\tli $v0, 1\n"); // Syscall code for print_int
        fprintf(outputFile, "\tsyscall\n");
        // Print newline character
        fprintf(outputFile, "\tli $a0, 10\n"); // ASCII code for newline
        fprintf(outputFile, "\tli $v0, 11\n"); // Syscall code for print_char
        fprintf(outputFile, "\tsyscall\n");
    }
    else if (strcmp(current->op, "write_float") == 0)
    {
        // Write operation for floating-point numbers
        fprintf(outputFile, "# Generating MIPS code for write_float operation\n");
        const char *srcReg = getFloatRegisterForVariable(current->arg1);
        if (!srcReg)
        {
            srcReg = allocateFloatRegister();
            setFloatRegisterForVariable(current->arg1, srcReg);
            loadFloatOperand(current->arg1, srcReg);
        }
        // Move value to $f12 for floating-point printing
        fprintf(outputFile, "\tmov.s $f12, %s\n", srcReg);
        fprintf(outputFile, "\tli $v0, 2\n"); // Syscall code for print_float
        fprintf(outputFile, "\tsyscall\n");

        // Print newline character after the float
        fprintf(outputFile, "\tli $a0, 10\n"); // ASCII code for newline
        fprintf(outputFile, "\tli $v0, 11\n"); // Syscall code for print_char
        fprintf(outputFile, "\tsyscall\n");
    }
    else if (strcmp(current->op, "[]=") == 0)
    {
        // Array assignment operation
        fprintf(outputFile, "# Generating MIPS code for array assignment\n");
        // Load base address of array into BASE_ADDRESS_REGISTER
        fprintf(outputFile, "\tla %s, %s\n", BASE_ADDRESS_REGISTER, current->result);
        // Compute offset if possible
        char *offsetValue = computeOffset(current->arg1, 4);
        if (offsetValue != NULL)
        {
            // Load value
            const char *valueReg = getRegisterForVariable(current->arg2);
            if (!valueReg)
            {
                valueReg = allocateRegister();
                if (!valueReg)
                {
                    fprintf(stderr, "Error: No available registers for value\n");
                    exit(1);
                }
                setRegisterForVariable(current->arg2, valueReg);
                loadOperand(current->arg2, valueReg);
            }
            fprintf(outputFile, "\tsw %s, %s(%s)\n", valueReg, offsetValue, BASE_ADDRESS_REGISTER);
            free(offsetValue);
        }
        else
        {
            // Index is variable, compute at runtime
            // Load index
            const char *indexReg = getRegisterForVariable(current->arg1);
            if (!indexReg)
            {
                indexReg = allocateRegister();
                if (!indexReg)
                {
                    fprintf(stderr, "Error: No available registers for index\n");
                    exit(1);
                }
                setRegisterForVariable(current->arg1, indexReg);
                loadOperand(current->arg1, indexReg);
            }
            // Load value
            const char *valueReg = getRegisterForVariable(current->arg2);
            if (!valueReg)
            {
                valueReg = allocateRegister();
                if (!valueReg)
                {
                    fprintf(stderr, "Error: No available registers for value\n");
                    exit(1);
                }
                setRegisterForVariable(current->arg2, valueReg);
                loadOperand(current->arg2, valueReg);
            }
            // Calculate offset: indexReg * 4
            const char *tempReg = ADDRESS_CALC_REGISTER;
            fprintf(outputFile, "\tmul %s, %s, 4\n", tempReg, indexReg);
            // Effective address: BASE_ADDRESS_REGISTER + tempReg
            fprintf(outputFile, "\tadd %s, %s, %s\n", tempReg, BASE_ADDRESS_REGISTER, tempReg);
            // Store value
            fprintf(outputFile, "\tsw %s, 0(%s)\n", valueReg, tempReg);
        }
    }
    else if (strcmp(current->op, "=[]") == 0)
    {
        // Array access operation
        fprintf(outputFile, "# Generating MIPS code for array access\n");
        // Load base address of array into BASE_ADDRESS_REGISTER
        fprintf(outputFile, "\tla %s, %s\n", BASE_ADDRESS_REGISTER, current->arg1);
        // Compute offset if possible
        char *offsetValue = computeOffset(current->arg2, 4);
        if (offsetValue != NULL)
        {
            // Load value into a register
            const char *resultReg = getRegisterForVariable(current->result);
            if (!resultReg)
            {
                resultReg = allocateRegister();
                if (!resultReg)
                {
                    fprintf(stderr, "Error: No available registers for result %s\n", current->result);
                    exit(1);
                }
                setRegisterForVariable(current->result, resultReg);
            }
            fprintf(outputFile, "\tlw %s, %s(%s)\n", resultReg, offsetValue, BASE_ADDRESS_REGISTER);
            free(offsetValue);
        }
        else
        {
            // Index is variable, compute at runtime
            // Load index
            const char *indexReg = getRegisterForVariable(current->arg2);
            if (!indexReg)
            {
                indexReg = allocateRegister();
                if (!indexReg)
                {
                    fprintf(stderr, "Error: No available registers for index\n");
                    exit(1);
                }
                setRegisterForVariable(current->arg2, indexReg);
                loadOperand(current->arg2, indexReg);
            }
            // Calculate offset: indexReg * 4
            const char *tempReg = ADDRESS_CALC_REGISTER;
            fprintf(outputFile, "\tmul %s, %s, 4\n", tempReg, indexReg);
            // Effective address: BASE_ADDRESS_REGISTER + tempReg
            fprintf(outputFile, "\tadd %s, %s, %s\n", tempReg, BASE_ADDRESS_REGISTER, tempReg);
            // Load value into a register
            const char *resultReg = getRegisterForVariable(current->result);
            if (!resultReg)
            {
                resultReg = allocateRegister();
                if (!resultReg)
                {
                    fprintf(stderr, "Error: No available registers for result %s\n", current->result);
                    exit(1);
                }
                setRegisterForVariable(current->result, resultReg);
            }
            fprintf(outputFile, "\tlw %s, 0(%s)\n", resultReg, tempReg);
        }
    }

    else if (strcmp(current->op, "fmov") == 0)
    {
        // Floating point move operation
        fprintf(outputFile, "# Generating MIPS code for floating point move\n");

        const char *srcReg = getFloatRegisterForVariable(current->arg1);
        if (!srcReg)
        {
            srcReg = allocateFloatRegister();
            setFloatRegisterForVariable(current->arg1, srcReg);

            // Load the float value from memory using the label (e.g., floatA)
            fprintf(outputFile, "\tl.s %s, %s\n", srcReg, current->result); // Use label instead of immediate value
        }

        const char *destReg = getFloatRegisterForVariable(current->result);
        if (!destReg)
        {
            destReg = allocateFloatRegister();
            setFloatRegisterForVariable(current->result, destReg);
        }

        // Move the floating-point value from srcReg to destReg
        fprintf(outputFile, "\tmov.s %s, %s\n", destReg, srcReg);
    }
    else
    {
        fprintf(stderr, "Warning: Unsupported TAC operation '%s'\n", current->op);
    }

    // Deallocate registers for variables no longer used
    const char *variablesToCheck[] = {current->arg1, current->arg2, current->result};
    for (int i = 0; i < 3; i++)
    {
        const char *var = variablesToCheck[i];

        // Deallocate integer registers
        if (var != NULL && isVariableInRegisterMap(var))
        {
            if (!isVariableUsedLater(current, var))
            {
                const char *regName = getRegisterForVariable(var);
                if (findSymbol(symTab, var))
                {
                    // Only store user-defined variables back to memory
                    fprintf(outputFile, "# Storing variable %s back to memory\n", var);
                    fprintf(outputFile, "\tsw %s, %s\n", regName, var);
                }
                deallocateRegister(regName);
                removeVariableFromRegisterMap(var);
            }
        }

        // Deallocate float registers
        if (var != NULL && isVariableInFloatRegisterMap(var))
        {
            if (!isVariableUsedLater(current, var))
            {
                const char *regName = getFloatRegisterForVariable(var);
                if (findSymbol(symTab, var))
                {
                    // Only store user-defined float variables back to memory
                    fprintf(outputFile, "# Storing float variable %s back to memory\n", var);
                    fprintf(outputFile, "\ts.s %s, %s\n", regName, var);
                }
                deallocateFloatRegister(regName);
                removeVariableFromFloatRegisterMap(var);
            }
        }
    }
}