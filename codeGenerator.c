// codeGenerator.c

#include "codeGenerator.h"
#include "optimizer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

static FILE *outputFile;

// Available registers (excluding $t8 and $t9)
const char *availableRegisters[] = {"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7"};
#define NUM_AVAILABLE_REGISTERS 8
bool registerInUse[NUM_AVAILABLE_REGISTERS] = {false};

// Reserved registers
#define ADDRESS_CALC_REGISTER "$t9"
#define BASE_ADDRESS_REGISTER "$t8"

// Register map to keep track of variable to register mappings
RegisterMapEntry registerMap[MAX_REGISTER_MAP_SIZE];

void initializeRegisterMap()
{
    for (int i = 0; i < MAX_REGISTER_MAP_SIZE; i++)
    {
        registerMap[i].variable = NULL;
        registerMap[i].regName = NULL;
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

void initCodeGenerator(const char *outputFilename)
{
    outputFile = fopen(outputFilename, "w");
    if (outputFile == NULL)
    {
        perror("Failed to open output file");
        exit(EXIT_FAILURE);
    }
    initializeRegisterMap();
}

void generateMIPS(TAC *tacInstructions, SymbolTable *symTab)
{
    VarNode *varList = NULL; // Declare and initialize varList

    // Collect variables from TAC instructions
    collectVariables(tacInstructions, &varList);

    // Generate the .data section
    fprintf(outputFile, ".data\n");

    // Declare variables from the symbol table
    for (int i = 0; i < symTab->size; i++)
    {
        Symbol *symbol = symTab->table[i];
        while (symbol != NULL)
        {
            if (symbol->isArray)
            {
                int totalSize = symbol->arrayInfo->size * 4; // Assuming 4 bytes per element
                fprintf(outputFile, "%s: .space %d\n", symbol->name, totalSize);
            }
            else
            {
                fprintf(outputFile, "%s: .word 0\n", symbol->name);
            }
            symbol = symbol->next;
        }
    }

    // Declare temporary variables not in symbol table
    VarNode *currentVar = varList;
    while (currentVar != NULL)
    {
        if (!findSymbol(symTab, currentVar->name))
        {
            if (!isConstant(currentVar->name))
            {
                fprintf(outputFile, "%s: .word 0\n", currentVar->name);
            }
        }
        currentVar = currentVar->next;
    }

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
                // Generate code for binary operations
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
                    // Index is constant, use immediate offset
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
                    // Index is constant, use immediate offset
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
        }

        current = current->next;
    }

    // Before exiting, store all live registers back to memory
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
    freeRegisterMap();

    // Exit program
    fprintf(outputFile, "\tli $v0, 10\n");
    fprintf(outputFile, "\tsyscall\n");

    // Free the variable list
    freeVariableList(varList);
}

void finalizeCodeGenerator(const char *outputFilename)
{
    if (outputFile)
    {
        fclose(outputFile);
        printf("MIPS code generated and saved to file %s\n", outputFilename);
        outputFile = NULL;
    }
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
    // No available register, implement spilling if necessary
    return NULL; // Indicate failure
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

// Function to check if a variable is already in the list
bool isVariableInList(VarNode *varList, const char *varName)
{
    VarNode *current = varList;
    while (current != NULL)
    {
        if (strcmp(current->name, varName) == 0)
        {
            return true;
        }
        current = current->next;
    }
    return false;
}

// Function to add a variable to the list
void addVariable(VarNode **varList, const char *varName, int initialValue, bool isInitialized)
{
    VarNode *existingNode = *varList;
    while (existingNode != NULL)
    {
        if (strcmp(existingNode->name, varName) == 0)
        {
            // Update initial value if variable already exists and is uninitialized
            if (!existingNode->isInitialized && isInitialized)
            {
                existingNode->initialValue = initialValue;
                existingNode->isInitialized = true;
            }
            return; // Variable already in the list
        }
        existingNode = existingNode->next;
    }
    // Variable not found, create a new one
    VarNode *newNode = malloc(sizeof(VarNode));
    newNode->name = strdup(varName);
    newNode->initialValue = initialValue;
    newNode->isInitialized = isInitialized;
    newNode->next = *varList;
    *varList = newNode;
}

// Function to collect all variables from TAC instructions
void collectVariables(TAC *tacInstructions, VarNode **varList)
{
    TAC *current = tacInstructions;
    while (current != NULL)
    {
        if (current->result != NULL)
        {
            addVariable(varList, current->result, 0, false);
        }
        if (current->arg1 != NULL)
        {
            addVariable(varList, current->arg1, 0, false);
        }
        if (current->arg2 != NULL)
        {
            addVariable(varList, current->arg2, 0, false);
        }
        current = current->next;
    }
}

// Function to free the variable list
void freeVariableList(VarNode *varList)
{
    VarNode *current = varList;
    while (current != NULL)
    {
        VarNode *next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
}

VarNode *findVariable(VarNode *varList, const char *varName)
{
    VarNode *current = varList;
    while (current != NULL)
    {
        if (strcmp(current->name, varName) == 0)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void loadOperand(const char *operand, const char *registerName)
{
    if (isConstant(operand))
    {
        fprintf(outputFile, "\tli %s, %s\n", registerName, operand);
    }
    else if (isVariableInRegisterMap(operand))
    {
        // Operand is in a register
        const char *reg = getRegisterForVariable(operand);
        if (strcmp(registerName, reg) != 0)
        {
            fprintf(outputFile, "\tmove %s, %s\n", registerName, reg);
        }
    }
    else
    {
        // Load variable from memory into register
        fprintf(outputFile, "\tlw %s, %s\n", registerName, operand);
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
