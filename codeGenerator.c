// codeGenerator.c

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
    for (int i = 0; i < MAX_FLOAT_REGISTER_MAP_SIZE; i++)
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
    const char *spillRegister = availableRegisters[0]; // Example: spill the first register
    fprintf(outputFile, "# Spilling register %s to memory\n", spillRegister);
    // Store spilled register's value in memory (e.g., on the stack)
    fprintf(outputFile, "\tsw %s, spill_area\n", spillRegister);

    return spillRegister;
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
    return NULL;
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

// Function to collect all symbols from all symbol tables
// void collectAllSymbols(SymbolTable *symTab, Symbol ***symbolList, int *symbolCount, int *symbolCapacity)
// {
//     SymbolTable *currentSymTab = symTab;
//     while (currentSymTab != NULL)
//     {
//         // Process symbols in currentSymTab
//         for (int i = 0; i < currentSymTab->size; i++)
//         {
//             Symbol *symbol = currentSymTab->table[i];
//             while (symbol != NULL)
//             {
//                 // Resize symbolList if necessary
//                 if (*symbolCount >= *symbolCapacity)
//                 {
//                     *symbolCapacity *= 2;
//                     *symbolList = realloc(*symbolList, (*symbolCapacity) * sizeof(Symbol *));
//                     if (*symbolList == NULL)
//                     {
//                         fprintf(stderr, "Error: Memory allocation failed while collecting symbols.\n");
//                         exit(1);
//                     }
//                 }
//                 // Add symbol to the symbolList
//                 (*symbolList)[*symbolCount] = symbol;
//                 (*symbolCount)++;
//                 symbol = symbol->next;
//             }
//         }
//         // Move to the next symbol table
//         currentSymTab = currentSymTab->next;
//     }
// }

void generateMIPS(TAC *tacInstructions, SymbolTable *symTab)
{
    // Collect all symbols from all symbol tables
    int symbolCapacity = 100; // Initial capacity, adjust as needed
    int symbolCount = 0;
    Symbol **symbolList = malloc(symbolCapacity * sizeof(Symbol *));
    if (symbolList == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for symbol list.\n");
        exit(1);
    }
    collectAllSymbols(symTab, &symbolList, &symbolCount, &symbolCapacity);

    // Generate the .data section
    fprintf(outputFile, ".data\n");

    // Declare the spill area for register spilling
    fprintf(outputFile, "spill_area: .word 0\n");

    // Declare variables from the collected symbols
    for (int i = 0; i < symbolCount; i++)
    {
        Symbol *symbol = symbolList[i];

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
        else
        {
            fprintf(outputFile, "%s: .word 0\n", symbol->name);
        }
    }

    // Free the symbolList after use
    free(symbolList);

    // Start the .text section and main function
    fprintf(outputFile, ".text\n");
    fprintf(outputFile, ".globl main\n");
    fprintf(outputFile, "main:\n");

    TAC *current = tacInstructions;
    while (current != NULL)
    {
        if (current->op != NULL)
        {
            if (strcmp(current->op, "+") == 0 || strcmp(current->op, "-") == 0 ||
                strcmp(current->op, "*") == 0 || strcmp(current->op, "/") == 0)
            {
                // Generate code for integer binary operations
                fprintf(outputFile, "# Generating MIPS code for operation %s\n", current->op);
                // Load operands
                const char *reg1 = getRegisterForVariable(current->arg1);
                if (!reg1)
                {
                    reg1 = allocateRegister();
                    if (!reg1)
                    {
                        fprintf(stderr, "Error: No available registers for operand %s\n", current->arg1);
                        exit(1);
                    }
                    setRegisterForVariable(current->arg1, reg1);
                    loadOperand(current->arg1, reg1);
                }
                const char *reg2 = getRegisterForVariable(current->arg2);
                if (!reg2)
                {
                    reg2 = allocateRegister();
                    if (!reg2)
                    {
                        fprintf(stderr, "Error: No available registers for operand %s\n", current->arg2);
                        exit(1);
                    }
                    setRegisterForVariable(current->arg2, reg2);
                    loadOperand(current->arg2, reg2);
                }
                const char *resultReg = allocateRegister();
                if (!resultReg)
                {
                    fprintf(stderr, "Error: No available registers for result %s\n", current->result);
                    exit(1);
                }
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
                // No need to store result to memory immediately
            }
            else if (strcmp(current->op, "fadd") == 0 || strcmp(current->op, "fsub") == 0 ||
                     strcmp(current->op, "fmul") == 0 || strcmp(current->op, "fdiv") == 0)
            {
                // Generate code for floating-point binary operations
                fprintf(outputFile, "# Generating MIPS code for floating-point operation %s\n", current->op);
                // Load operands
                const char *reg1 = getFloatRegisterForVariable(current->arg1);
                if (!reg1)
                {
                    reg1 = allocateFloatRegister();
                    if (!reg1)
                    {
                        fprintf(stderr, "Error: No available float registers for operand %s\n", current->arg1);
                        exit(1);
                    }
                    setFloatRegisterForVariable(current->arg1, reg1);
                    loadFloatOperand(current->arg1, reg1);
                }
                const char *reg2 = getFloatRegisterForVariable(current->arg2);
                if (!reg2)
                {
                    reg2 = allocateFloatRegister();
                    if (!reg2)
                    {
                        fprintf(stderr, "Error: No available float registers for operand %s\n", current->arg2);
                        exit(1);
                    }
                    setFloatRegisterForVariable(current->arg2, reg2);
                    loadFloatOperand(current->arg2, reg2);
                }
                const char *resultReg = allocateFloatRegister();
                if (!resultReg)
                {
                    fprintf(stderr, "Error: No available float registers for result %s\n", current->result);
                    exit(1);
                }
                setFloatRegisterForVariable(current->result, resultReg);
                // Perform operation
                if (strcmp(current->op, "fadd") == 0)
                {
                    fprintf(outputFile, "\tadd.s %s, %s, %s\n", resultReg, reg1, reg2);
                }
                else if (strcmp(current->op, "fsub") == 0)
                {
                    fprintf(outputFile, "\tsub.s %s, %s, %s\n", resultReg, reg1, reg2);
                }
                else if (strcmp(current->op, "fmul") == 0)
                {
                    fprintf(outputFile, "\tmul.s %s, %s, %s\n", resultReg, reg1, reg2);
                }
                else if (strcmp(current->op, "fdiv") == 0)
                {
                    fprintf(outputFile, "\tdiv.s %s, %s, %s\n", resultReg, reg1, reg2);
                }
                // No need to store result to memory immediately
            }
            else if (strcmp(current->op, "=") == 0)
            {
                // Assignment operation
                fprintf(outputFile, "# Generating MIPS code for assignment\n");
                const char *srcReg = getRegisterForVariable(current->arg1);
                if (!srcReg)
                {
                    srcReg = allocateRegister();
                    if (!srcReg)
                    {
                        fprintf(stderr, "Error: No available registers for operand %s\n", current->arg1);
                        exit(1);
                    }
                    setRegisterForVariable(current->arg1, srcReg);
                    loadOperand(current->arg1, srcReg);
                }
                // Map result variable to a register
                const char *destReg = getRegisterForVariable(current->result);
                if (!destReg)
                {
                    destReg = allocateRegister();
                    if (!destReg)
                    {
                        fprintf(stderr, "Error: No available registers for result %s\n", current->result);
                        exit(1);
                    }
                    setRegisterForVariable(current->result, destReg);
                }
                fprintf(outputFile, "\tmove %s, %s\n", destReg, srcReg);
                // No need to store to memory immediately
            }
            else if (strcmp(current->op, "fmov") == 0)
            {
                // Floating-point assignment operation
                fprintf(outputFile, "# Generating MIPS code for floating-point assignment\n");
                const char *srcReg = getFloatRegisterForVariable(current->arg1);
                if (!srcReg)
                {
                    srcReg = allocateFloatRegister();
                    if (!srcReg)
                    {
                        fprintf(stderr, "Error: No available float registers for operand %s\n", current->arg1);
                        exit(1);
                    }
                    setFloatRegisterForVariable(current->arg1, srcReg);
                    loadFloatOperand(current->arg1, srcReg);
                }
                // Map result variable to a float register
                const char *destReg = getFloatRegisterForVariable(current->result);
                if (!destReg)
                {
                    destReg = allocateFloatRegister();
                    if (!destReg)
                    {
                        fprintf(stderr, "Error: No available float registers for result %s\n", current->result);
                        exit(1);
                    }
                    setFloatRegisterForVariable(current->result, destReg);
                }
                fprintf(outputFile, "\tmov.s %s, %s\n", destReg, srcReg);
                // No need to store to memory immediately
            }
            else if (strcmp(current->op, "write") == 0)
            {
                // Write operation
                fprintf(outputFile, "# Generating MIPS code for write operation\n");
                const char *srcReg = getRegisterForVariable(current->arg1);
                if (!srcReg)
                {
                    // Load operand into $a0 directly if it's not in a register
                    loadOperand(current->arg1, "$a0");
                }
                else
                {
                    // Move value to $a0
                    fprintf(outputFile, "\tmove $a0, %s\n", srcReg);
                }
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
                    // Load operand into $f12 directly if it's not in a register
                    loadFloatOperand(current->arg1, "$f12");
                }
                else
                {
                    // Move value to $f12 for floating-point printing
                    fprintf(outputFile, "\tmov.s $f12, %s\n", srcReg);
                }
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
            else
            {
                fprintf(stderr, "Warning: Unsupported TAC operation '%s'\n", current->op);
            }
        }

        // Deallocate registers for variables no longer used
        const char *variablesToCheck[] = {current->arg1, current->arg2, current->result};
        for (int i = 0; i < 3; i++)
        {
            const char *var = variablesToCheck[i];
            if (var != NULL && isVariableInRegisterMap(var))
            {
                if (!isVariableUsedLater(current, var))
                {
                    const char *regName = getRegisterForVariable(var);
                    // Store the variable back to memory if it's a user-defined variable or a temporary variable
                    if (findSymbol(symTab, var) || isTemporaryVariable(var))
                    {
                        fprintf(outputFile, "# Storing variable %s back to memory\n", var);
                        fprintf(outputFile, "\tsw %s, %s\n", regName, var);
                    }
                    deallocateRegister(regName);
                    removeVariableFromRegisterMap(var);
                }
            }
            if (var != NULL && isVariableInFloatRegisterMap(var))
            {
                if (!isVariableUsedLater(current, var))
                {
                    const char *regName = getFloatRegisterForVariable(var);
                    // Store the variable back to memory if it's a user-defined variable or a temporary variable
                    if (findSymbol(symTab, var) || isTemporaryVariable(var))
                    {
                        fprintf(outputFile, "# Storing float variable %s back to memory\n", var);
                        fprintf(outputFile, "\ts.s %s, %s\n", regName, var);
                    }
                    deallocateFloatRegister(regName);
                    removeVariableFromFloatRegisterMap(var);
                }
            }
        }

        current = current->next;
    }

    // Before exiting, store all live integer registers back to memory
    for (int i = 0; i < MAX_REGISTER_MAP_SIZE; i++)
    {
        if (registerMap[i].variable != NULL)
        {
            const char *var = registerMap[i].variable;
            const char *regName = registerMap[i].regName;
            if (findSymbol(symTab, var) || isTemporaryVariable(var))
            {
                fprintf(outputFile, "# Storing variable %s back to memory\n", var);
                fprintf(outputFile, "\tsw %s, %s\n", regName, var);
            }
            deallocateRegister(regName);
            free(registerMap[i].variable);
            free(registerMap[i].regName);
            registerMap[i].variable = NULL;
            registerMap[i].regName = NULL;
        }
    }

    // Before exiting, store all live float registers back to memory
    for (int i = 0; i < MAX_FLOAT_REGISTER_MAP_SIZE; i++)
    {
        if (floatRegisterMap[i].variable != NULL)
        {
            const char *var = floatRegisterMap[i].variable;
            const char *regName = floatRegisterMap[i].regName;
            if (findSymbol(symTab, var) || isTemporaryVariable(var))
            {
                fprintf(outputFile, "# Storing float variable %s back to memory\n", var);
                fprintf(outputFile, "\ts.s %s, %s\n", regName, var);
            }
            deallocateFloatRegister(regName);
            free(floatRegisterMap[i].variable);
            free(floatRegisterMap[i].regName);
            floatRegisterMap[i].variable = NULL;
            floatRegisterMap[i].regName = NULL;
        }
    }

    // Exit program
    fprintf(outputFile, "\tli $v0, 10\n");
    fprintf(outputFile, "\tsyscall\n");
}