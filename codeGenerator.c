#include "codeGenerator.h"
#include "SymbolTable.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

static FILE *outputFile;

const char *availableRegisters[] = {"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7"};
#define NUM_AVAILABLE_REGISTERS 8
bool registerInUse[NUM_AVAILABLE_REGISTERS] = {false};

const char *availableFloatRegisters[] = {"$f0", "$f2", "$f4", "$f6", "$f8", "$f10", "$f12", "$f14"};
#define NUM_AVAILABLE_FLOAT_REGISTERS 8
bool floatRegisterInUse[NUM_AVAILABLE_FLOAT_REGISTERS] = {false};

#define ADDRESS_CALC_REGISTER "$t9"
#define BASE_ADDRESS_REGISTER "$t8"

#define MAX_REGISTER_MAP_SIZE 100
RegisterMapEntry registerMap[MAX_REGISTER_MAP_SIZE];

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
    initializeFloatRegisterMap();
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
    exit(1);
}

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

const char *getRegisterForVariable(const char *variable)
{
    for (int i = 0; i < MAX_REGISTER_MAP_SIZE; i++)
    {
        if (registerMap[i].variable != NULL && strcmp(registerMap[i].variable, variable) == 0)
        {
            return registerMap[i].regName;
        }
    }
    return NULL;
}

bool isVariableInRegisterMap(const char *variable)
{
    return getRegisterForVariable(variable) != NULL;
}

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
    exit(1);
}

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

bool isVariableInFloatRegisterMap(const char *variable)
{
    return getFloatRegisterForVariable(variable) != NULL;
}

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
    return NULL;
}

void loadOperand(const char *operand, const char *registerName)
{
    if (isConstant(operand))
    {
        if (registerName[0] == '$' && registerName[1] == 'f')
        {
            fprintf(outputFile, "\tli.s %s, %s\n", registerName, operand);
        }
        else
        {
            fprintf(outputFile, "\tli %s, %s\n", registerName, operand);
        }
    }
    else if (isVariableInRegisterMap(operand))
    {
        const char *reg = getRegisterForVariable(operand);
        if (strcmp(registerName, reg) != 0)
        {
            fprintf(outputFile, "\tmove %s, %s\n", registerName, reg);
        }
    }
    else if (isVariableInFloatRegisterMap(operand))
    {
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
        // Assume integer load from memory by default
        fprintf(outputFile, "\tlw %s, %s\n", registerName, operand);
    }
}

void loadFloatOperand(const char *operand, const char *reg)
{
    if (isConstant(operand))
    {
        fprintf(outputFile, "\tli.s %s, %s\n", reg, operand);
    }
    else if (isVariableInFloatRegisterMap(operand))
    {
        const char *srcReg = getFloatRegisterForVariable(operand);
        if (strcmp(reg, srcReg) != 0)
        {
            fprintf(outputFile, "\tmov.s %s, %s\n", reg, srcReg);
        }
    }
    else
    {
        fprintf(outputFile, "\tl.s %s, %s\n", reg, operand);
    }
}

bool isTemporaryVariable(const char *operand)
{
    if (operand == NULL)
        return false;
    if (operand[0] != 't')
        return false;
    for (int i = 1; operand[i] != '\0'; i++)
    {
        if (!isdigit((unsigned char)operand[i]))
        {
            return false;
        }
    }
    return true;
}

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
    int symbolCapacity = 100;
    int symbolCount = 0;
    Symbol **symbolList = malloc(symbolCapacity * sizeof(Symbol *));
    collectAllSymbols(symTab, &symbolList, &symbolCount, &symbolCapacity);

    fprintf(outputFile, ".data\n");
    fprintf(outputFile, "spill_area: .word 0\n");

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

        declaredVariables[declaredCount++] = strdup(symbol->name);

        if (symbol->isArray)
        {
            int totalSize = symbol->arrayInfo->size * 4;
            fprintf(outputFile, "%s: .space %d\n", symbol->name, totalSize);
        }
        else if (strcmp(symbol->type, "float") == 0)
        {
            if (symbol->value != NULL)
            {
                fprintf(outputFile, "%s: .float %s\n", symbol->name, symbol->value);
            }
            else
            {
                fprintf(outputFile, "%s: .float 0.0\n", symbol->name);
            }
        }
        else if (strcmp(symbol->type, "int") == 0)
        {
            if (symbol->value != NULL)
            {
                fprintf(outputFile, "%s: .word %s\n", symbol->name, symbol->value);
            }
            else
            {
                fprintf(outputFile, "%s: .word 0\n", symbol->name);
            }
        }
        else if (strcmp(symbol->type, "bool") == 0)
        {
            if (symbol->value != NULL)
            {
                // Convert "true"/"false" to 1/0
                int boolValue = (strcmp(symbol->value, "true") == 0) ? 1 : 0;
                fprintf(outputFile, "%s: .word %d\n", symbol->name, boolValue);
            }
            else
            {
                fprintf(outputFile, "%s: .word 0\n", symbol->name);
            }
        }
    }

    for (int i = 0; i < declaredCount; i++)
    {
        free(declaredVariables[i]);
    }
    free(declaredVariables);
    free(symbolList);

    fprintf(outputFile, ".text\n");

    TAC *current = tacInstructions;
    TAC *mainTAC = NULL;
    TAC *mainTACTail = NULL;
    TAC *otherFunctionsTAC = NULL;
    TAC *otherFunctionsTACTail = NULL;
    const char *currentFunctionName = NULL;
    bool inMainFunction = false;
    bool inFunction = false;

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

        if (inFunction)
        {
            if (inMainFunction)
            {
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

    if (mainTACTail != NULL)
    {
        mainTACTail->next = NULL;
    }
    if (otherFunctionsTACTail != NULL)
    {
        otherFunctionsTACTail->next = NULL;
    }

    processTACList(mainTAC, symTab);

    fprintf(outputFile, "\n");

    processTACList(otherFunctionsTAC, symTab);
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

                inFunction = true;
            }
            else if (strcmp(current->op, "prologue") == 0)
            {
                fprintf(outputFile, "# Prologue for function %s\n", currentFunctionName);
                fprintf(outputFile, "\taddiu $sp, $sp, -8\n");
                fprintf(outputFile, "\tsw $fp, 4($sp)\n");
                fprintf(outputFile, "\tsw $ra, 0($sp)\n");
                fprintf(outputFile, "\tmove $fp, $sp\n");

                initializeRegisterMap();
                initializeFloatRegisterMap();
                memset(registerInUse, 0, sizeof(registerInUse));
                memset(floatRegisterInUse, 0, sizeof(floatRegisterInUse));
            }
            else if (strcmp(current->op, "epilogue") == 0)
            {
                fprintf(outputFile, "# Epilogue for function %s\n", currentFunctionName);
                fprintf(outputFile, "%s_epilogue:\n", currentFunctionName);

                fprintf(outputFile, "\tmove $sp, $fp\n");
                fprintf(outputFile, "\tlw $fp, 4($sp)\n");
                fprintf(outputFile, "\tlw $ra, 0($sp)\n");
                fprintf(outputFile, "\taddiu $sp, $sp, 8\n");

                if (strcmp(currentFunctionName, "main") == 0)
                {
                    fprintf(outputFile, "\tli $v0, 10\n");
                    fprintf(outputFile, "\tsyscall\n");
                }
                else
                {
                    fprintf(outputFile, "\tjr $ra\n");
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

                generateTACOperation(current, symTab, currentFunctionName);
            }
        }

        current = current->next;
    }
}

static int currentArgCount = 0;

void handleFunctionArguments(TAC *current, int *argCount)
{
    if (strcmp(current->op, "param") != 0)
    {
        *argCount = 0;
        return;
    }

    const char *argReg;
    switch (*argCount)
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
        fprintf(stderr, "Warning: More than 4 arguments not supported\n");
        return;
    }

    if (current->arg2 && strcmp(current->arg2, "float") == 0)
    {
        fprintf(outputFile, "\tl.s $f12, %s\n", current->arg1);
    }
    else
    {
        const char *srcReg = getRegisterForVariable(current->arg1);
        if (!srcReg)
        {
            srcReg = allocateRegister();
            setRegisterForVariable(current->arg1, srcReg);
            loadOperand(current->arg1, srcReg);
        }
        fprintf(outputFile, "\tmove %s, %s\n", argReg, srcReg);
    }
    (*argCount)++;
}

void generateTACOperation(TAC *current, SymbolTable *symTab, const char *currentFunctionName)
{
    if (current->op == NULL)
    {
        return;
    }

    if (strcmp(current->op, "return") == 0)
    {
        fprintf(outputFile, "# Return from function %s\n", currentFunctionName);
        if (current->arg1 != NULL)
        {
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
        fprintf(outputFile, "\tj %s_epilogue\n", currentFunctionName);
    }
    else if (strcmp(current->op, "call") == 0)
    {
        fprintf(outputFile, "# Function call to %s\n", current->arg1);
        fprintf(outputFile, "\tjal %s\n", current->arg1);
        currentArgCount = 0; // Reset argument counter after function call
    }
    else if (strcmp(current->op, "=") == 0)
    {
        fprintf(outputFile, "# Assignment\n");

        // If arg1 == "v0" we treat it as return value from function call
        if (strcmp(current->arg1, "v0") == 0)
        {
            // Result must be placed into a register
            const char *destReg = getRegisterForVariable(current->result);
            if (!destReg)
            {
                destReg = allocateRegister();
                setRegisterForVariable(current->result, destReg);
            }
            fprintf(outputFile, "\tmove %s, $v0\n", destReg);
        }
        else
        {
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
    }
    else if (strcmp(current->op, "+") == 0 ||
             strcmp(current->op, "-") == 0 ||
             strcmp(current->op, "*") == 0 ||
             strcmp(current->op, "/") == 0)
    {
        fprintf(outputFile, "# Integer arithmetic %s\n", current->op);
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
        if (strcmp(current->arg1, "v0") == 0)
        {
            // If we're writing the return value from a function
            fprintf(outputFile, "\tmove $a0, $v0\n");
        }
        else
        {
            const char *srcReg = getRegisterForVariable(current->arg1);
            if (!srcReg)
            {
                // If not in register, load from memory
                fprintf(outputFile, "\tlw $a0, %s\n", current->arg1);
            }
            else
            {
                // Move from register to $a0
                fprintf(outputFile, "\tmove $a0, %s\n", srcReg);
            }
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

        // Check if we're writing a return value from a function
        if (strcmp(current->arg1, "v0") == 0)
        {
            fprintf(outputFile, "\tmov.s $f12, $f0\n");
        }
        // Check if it's a temporary variable (starts with 't')
        else if (current->arg1[0] == 't')
        {
            const char *srcReg = getFloatRegisterForVariable(current->arg1);
            if (srcReg)
            {
                fprintf(outputFile, "\tmov.s $f12, %s\n", srcReg);
            }
            else
            {
                // If not in a register, try to load from memory
                fprintf(outputFile, "\tl.s $f12, sum\n"); // Assuming it's stored in sum
            }
        }
        // Regular variable
        else
        {
            const char *srcReg = getFloatRegisterForVariable(current->arg1);
            if (!srcReg)
            {
                // Load directly from memory location
                fprintf(outputFile, "\tl.s $f12, %s\n", current->arg1);
            }
            else
            {
                fprintf(outputFile, "\tmov.s $f12, %s\n", srcReg);
            }
        }

        fprintf(outputFile, "\tli $v0, 2\n"); // Syscall code for print_float
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
        fprintf(outputFile, "\tla %s, %s\n", BASE_ADDRESS_REGISTER, current->arg1);
        // Compute offset if possible
        char *offsetValue = computeOffset(current->arg2, 4);
        if (offsetValue != NULL)
        {
            // Ensure temporary variable is in the symbol table
            if (isTemporaryVariable(current->arg2))
            {
                if (!findSymbol(symTab, current->arg2))
                {
                    insertSymbol(symTab, current->arg2, "int", false, NULL, NULL);
                }
            }
            // Load value
            const char *valueReg = getRegisterForVariable(current->result);
            if (!valueReg)
            {
                valueReg = allocateRegister();
                if (!valueReg)
                {
                    fprintf(stderr, "Error: No available registers for value\n");
                    exit(1);
                }
                setRegisterForVariable(current->result, valueReg);
                loadOperand(current->result, valueReg);
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
        fprintf(outputFile, "# Array load\n");
        fprintf(outputFile, "\tla %s, %s\n", BASE_ADDRESS_REGISTER, current->arg1);

        char *offsetValue = computeOffset(current->arg2, 4);
        const char *resultReg = getRegisterForVariable(current->result);
        if (!resultReg)
        {
            resultReg = allocateRegister();
            setRegisterForVariable(current->result, resultReg);
        }

        if (offsetValue != NULL)
        {
            fprintf(outputFile, "\tlw %s, %s(%s)\n", resultReg, offsetValue, BASE_ADDRESS_REGISTER);
            free(offsetValue);
        }
        else
        {
            const char *indexReg = getRegisterForVariable(current->arg2);
            if (!indexReg)
            {
                indexReg = allocateRegister();
                setRegisterForVariable(current->arg2, indexReg);
                loadOperand(current->arg2, indexReg);
            }
            fprintf(outputFile, "\tmul %s, %s, 4\n", ADDRESS_CALC_REGISTER, indexReg);
            fprintf(outputFile, "\tadd %s, %s, %s\n", ADDRESS_CALC_REGISTER, BASE_ADDRESS_REGISTER, ADDRESS_CALC_REGISTER);
            fprintf(outputFile, "\tlw %s, 0(%s)\n", resultReg, ADDRESS_CALC_REGISTER);
        }
    }
    else if (strcmp(current->op, "param") == 0)
    {
        handleFunctionArguments(current, &currentArgCount);
    }
    else if (strcmp(current->op, "ifFalse") == 0)
    {
        fprintf(outputFile, "# ifFalse branch\n");
        const char *condReg = getRegisterForVariable(current->arg1);
        if (!condReg)
        {
            condReg = allocateRegister();
            setRegisterForVariable(current->arg1, condReg);
            loadOperand(current->arg1, condReg);
        }

        // Branch if zero
        fprintf(outputFile, "\tbeq %s, $zero, %s\n", condReg, current->result);
    }
    else if (strcmp(current->op, "goto") == 0)
    {
        fprintf(outputFile, "# Unconditional goto\n");
        fprintf(outputFile, "\tj %s\n", current->arg1);
    }
    else if (strcmp(current->op, "label") == 0)
    {
        // Handled in processTACList
    }
    else if (strcmp(current->op, "!") == 0)
    {
        fprintf(outputFile, "# Logical NOT\n");
        const char *exprReg = getRegisterForVariable(current->arg1);
        if (!exprReg)
        {
            exprReg = allocateRegister();
            setRegisterForVariable(current->arg1, exprReg);
            loadOperand(current->arg1, exprReg);
        }
        const char *resultReg = allocateRegister();
        setRegisterForVariable(current->result, resultReg);

        // NOT: result = (expr == 0) ? 1 : 0
        // Use a sequence:
        //  seq resultReg, exprReg, $zero
        fprintf(outputFile, "\tseq %s, %s, $zero\n", resultReg, exprReg);
    }
    else if (strcmp(current->op, "&&") == 0 || strcmp(current->op, "||") == 0)
    {
        fprintf(outputFile, "# Logical operation %s\n", current->op);
        const char *leftReg = getRegisterForVariable(current->arg1);
        if (!leftReg)
        {
            leftReg = allocateRegister();
            setRegisterForVariable(current->arg1, leftReg);
            loadOperand(current->arg1, leftReg);
        }
        const char *rightReg = getRegisterForVariable(current->arg2);
        if (!rightReg)
        {
            rightReg = allocateRegister();
            setRegisterForVariable(current->arg2, rightReg);
            loadOperand(current->arg2, rightReg);
        }
        const char *resultReg = allocateRegister();
        setRegisterForVariable(current->result, resultReg);

        if (strcmp(current->op, "&&") == 0)
        {
            // AND: use sne + seq or a direct approach:
            // set resultReg to 1 if both != 0:
            // first convert leftReg to boolean (neq zero), rightReg to boolean (neq zero)
            fprintf(outputFile, "\tsne %s, %s, $zero\n", leftReg, leftReg);
            fprintf(outputFile, "\tsne %s, %s, $zero\n", rightReg, rightReg);
            fprintf(outputFile, "\tand %s, %s, %s\n", resultReg, leftReg, rightReg);
        }
        else if (strcmp(current->op, "||") == 0)
        {
            // OR:
            fprintf(outputFile, "\tsne %s, %s, $zero\n", leftReg, leftReg);
            fprintf(outputFile, "\tsne %s, %s, $zero\n", rightReg, rightReg);
            fprintf(outputFile, "\tor %s, %s, %s\n", resultReg, leftReg, rightReg);
        }
    }
    else if (strcmp(current->op, "==") == 0 || strcmp(current->op, "!=") == 0 ||
             strcmp(current->op, "<") == 0 || strcmp(current->op, "<=") == 0 ||
             strcmp(current->op, ">") == 0 || strcmp(current->op, ">=") == 0)
    {
        fprintf(outputFile, "# Relational operation %s\n", current->op);
        const char *leftReg = getRegisterForVariable(current->arg1);
        if (!leftReg)
        {
            leftReg = allocateRegister();
            setRegisterForVariable(current->arg1, leftReg);
            loadOperand(current->arg1, leftReg);
        }
        const char *rightReg = getRegisterForVariable(current->arg2);
        if (!rightReg)
        {
            rightReg = allocateRegister();
            setRegisterForVariable(current->arg2, rightReg);
            loadOperand(current->arg2, rightReg);
        }
        const char *resultReg = allocateRegister();
        setRegisterForVariable(current->result, resultReg);

        if (strcmp(current->op, "==") == 0)
        {
            fprintf(outputFile, "\tseq %s, %s, %s\n", resultReg, leftReg, rightReg);
        }
        else if (strcmp(current->op, "!=") == 0)
        {
            fprintf(outputFile, "\tsne %s, %s, %s\n", resultReg, leftReg, rightReg);
        }
        else if (strcmp(current->op, "<") == 0)
        {
            fprintf(outputFile, "\tslt %s, %s, %s\n", resultReg, leftReg, rightReg);
        }
        else if (strcmp(current->op, "<=") == 0)
        {
            // <= is not a direct instruction: do !(left > right)
            // slt -> 1 if left < right
            // seq to check equality: left == right
            // or combine: sle = (left < right) or (left == right)
            const char *tempReg = allocateRegister();
            fprintf(outputFile, "\tslt %s, %s, %s\n", tempReg, leftReg, rightReg);
            const char *tempReg2 = allocateRegister();
            fprintf(outputFile, "\tseq %s, %s, %s\n", tempReg2, leftReg, rightReg);
            fprintf(outputFile, "\tor %s, %s, %s\n", resultReg, tempReg, tempReg2);
            deallocateRegister(tempReg);
            deallocateRegister(tempReg2);
        }
        else if (strcmp(current->op, ">") == 0)
        {
            // > is opposite of <=
            // slt resultReg, rightReg, leftReg
            fprintf(outputFile, "\tslt %s, %s, %s\n", resultReg, rightReg, leftReg);
        }
        else if (strcmp(current->op, ">=") == 0)
        {
            // >= is !(left < right)
            const char *tempReg = allocateRegister();
            fprintf(outputFile, "\tslt %s, %s, %s\n", tempReg, leftReg, rightReg);
            // now NOT it:
            fprintf(outputFile, "\tseq %s, %s, $zero\n", resultReg, tempReg);
            deallocateRegister(tempReg);
        }
    }
    else
    {
        fprintf(stderr, "Warning: Unsupported TAC operation '%s'\n", current->op);
    }

    const char *variablesToCheck[] = {current->arg1, current->arg2, current->result};
    for (int i = 0; i < 3; i++)
    {
        const char *var = variablesToCheck[i];
        if (var != NULL && isVariableInRegisterMap(var))
        {
            if (!isVariableUsedLater(current, var))
            {
                const char *regName = getRegisterForVariable(var);
                if (findSymbol(symTab, var))
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
                if (findSymbol(symTab, var))
                {
                    fprintf(outputFile, "# Storing float variable %s back to memory\n", var);
                    fprintf(outputFile, "\ts.s %s, %s\n", regName, var);
                }
                deallocateFloatRegister(regName);
                removeVariableFromFloatRegisterMap(var);
            }
        }
    }
}