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

static int currentArgCount = 0;

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

bool isFloatConstant(const char *operand)
{
    if (!operand) return false;
    // If it's a constant and contains a '.', treat as float constant
    if (isConstant(operand) && strchr(operand, '.'))
        return true;
    return false;
}

bool isFloatVariable(const char *variable, SymbolTable *symTab)
{
    if (!variable) return false;
    Symbol *sym = findSymbol(symTab, variable);
    if (sym && strcmp(sym->type, "float") == 0)
        return true;
    return false;
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

bool isFloatTypeOperand(const char *operand, SymbolTable *symTab)
{
    if (!operand) return false;
    if (isConstant(operand))
        return isFloatConstant(operand);
    return isFloatVariable(operand, symTab);
}

const char *ensureFloatInRegister(const char *operand, SymbolTable *symTab)
{
    // Load operand into a float register if not already
    if (isVariableInFloatRegisterMap(operand))
    {
        return getFloatRegisterForVariable(operand);
    }
    else if (isFloatConstant(operand))
    {
        const char *freg = allocateFloatRegister();
        fprintf(outputFile, "\tli.s %s, %s\n", freg, operand);
        setFloatRegisterForVariable(operand, freg);
        return freg;
    }
    else if (isVariableInRegisterMap(operand))
    {
        // Convert int register to float
        const char *intReg = getRegisterForVariable(operand);
        const char *fReg = allocateFloatRegister();
        // Move int to float: first move int to $fX using mtc1 and cvt.s.w
        fprintf(outputFile, "\tmtc1 %s, %s\n", intReg, fReg);
        fprintf(outputFile, "\tcvt.s.w %s, %s\n", fReg, fReg);
        setFloatRegisterForVariable(operand, fReg);
        return fReg;
    }
    else
    {
        // Load from memory as float
        const char *freg = allocateFloatRegister();
        fprintf(outputFile, "\tl.s %s, %s\n", freg, operand);
        setFloatRegisterForVariable(operand, freg);
        return freg;
    }
}

const char *ensureIntInRegister(const char *operand, SymbolTable *symTab)
{
    // Load operand into an int register
    if (isVariableInRegisterMap(operand))
    {
        return getRegisterForVariable(operand);
    }
    else if (isConstant(operand))
    {
        const char *reg = allocateRegister();
        fprintf(outputFile, "\tli %s, %s\n", reg, operand);
        setRegisterForVariable(operand, reg);
        return reg;
    }
    else if (isVariableInFloatRegisterMap(operand))
    {
        // Convert float to int
        const char *freg = getFloatRegisterForVariable(operand);
        const char *intReg = allocateRegister();
        // cvt.w.s to convert float in freg to int in freg, then move to intReg
        fprintf(outputFile, "\tcvt.w.s %s, %s\n", freg, freg);
        fprintf(outputFile, "\tmfc1 %s, %s\n", intReg, freg);
        // Since we're now int, remove from float map and put in int map
        removeVariableFromFloatRegisterMap(operand);
        setRegisterForVariable(operand, intReg);
        return intReg;
    }
    else
    {
        // Load from memory as int
        const char *reg = allocateRegister();
        fprintf(outputFile, "\tlw %s, %s\n", reg, operand);
        setRegisterForVariable(operand, reg);
        return reg;
    }
}

void loadOperandAsFloat(const char *operand, const char *freg, SymbolTable *symTab)
{
    if (isFloatConstant(operand))
    {
        fprintf(outputFile, "\tli.s %s, %s\n", freg, operand);
    }
    else if (isFloatVariable(operand, symTab))
    {
        // If variable is float but not in a register map
        if (isVariableInFloatRegisterMap(operand))
        {
            const char *src = getFloatRegisterForVariable(operand);
            if (strcmp(src, freg) != 0)
                fprintf(outputFile, "\tmov.s %s, %s\n", freg, src);
        }
        else
        {
            fprintf(outputFile, "\tl.s %s, %s\n", freg, operand);
        }
    }
    else
    {
        // If it's int but we want float
        const char *intReg = ensureIntInRegister(operand, symTab);
        fprintf(outputFile, "\tmtc1 %s, %s\n", intReg, freg);
        fprintf(outputFile, "\tcvt.s.w %s, %s\n", freg, freg);
    }
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

void handleFunctionArguments(TAC *current, int *argCount)
{
    // Simplified argument passing: Just move to a0-a3 or f12
    // Already implemented above, no float modifications needed here
    const char *argReg;
    switch (*argCount) {
        case 0: argReg = "$a0"; break;
        case 1: argReg = "$a1"; break;
        case 2: argReg = "$a2"; break;
        case 3: argReg = "$a3"; break;
        default:
            fprintf(stderr, "Warning: More than 4 arguments not supported\n");
            return;
    }

    // Check type
    if (current->arg2 && strcmp(current->arg2, "float") == 0) {
        // load as float
        const char *fReg = allocateFloatRegister();
        fprintf(outputFile, "\tl.s %s, %s\n", fReg, current->arg1);
        // move float arg to $f12 for now
        if (*argCount == 0) {
            fprintf(outputFile, "\tmov.s $f12, %s\n", fReg);
        } else {
            fprintf(stderr, "Warning: Only first float argument supported directly\n");
        }
        deallocateFloatRegister(fReg);
    } else {
        const char *srcReg = getRegisterForVariable(current->arg1);
        if (!srcReg) {
            // int arg
            // load as int
            // Actually handle int arg:
            srcReg = allocateRegister();
            setRegisterForVariable(current->arg1, srcReg);
            fprintf(outputFile, "\tlw %s, %s\n", srcReg, current->arg1);
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

    // Due to length, we focus on float fixes:
    // We'll detect float ops in arithmetic (+, -, *, /):
    // If any operand is float, we do float arithmetic.

    if (strcmp(current->op, "return") == 0)
    {
        fprintf(outputFile, "# Return from function %s\n", currentFunctionName);
        if (current->arg1 != NULL)
        {
            // Check if float or int
            if (isFloatTypeOperand(current->arg1, symTab))
            {
                const char *fReg = ensureFloatInRegister(current->arg1, symTab);
                // Convert float in fReg to int in v0 if needed or just store float in $f0 if you want to pass floats in f0
                // MIPS calling convention often returns float in $f0
                fprintf(outputFile, "\tmov.s $f0, %s\n", fReg);
                // If you want float return in $f0, that's fine. 
                // If you want int, convert here. We'll assume float return in $f0 is okay.
            }
            else
            {
                const char *reg = ensureIntInRegister(current->arg1, symTab);
                fprintf(outputFile, "\tmove $v0, %s\n", reg);
            }
        }
        fprintf(outputFile, "\tj %s_epilogue\n", currentFunctionName);
    }
    else if (strcmp(current->op, "call") == 0)
    {
        fprintf(outputFile, "# Function call to %s\n", current->arg1);
        fprintf(outputFile, "\tjal %s\n", current->arg1);
        currentArgCount = 0;  // Reset argument counter after function call
    }
    else if (strcmp(current->op, "=") == 0)
    {
        fprintf(outputFile, "# Assignment\n");
        // Determine if destination is float
        bool destFloat = isFloatVariable(current->result, symTab);

        if (strcmp(current->arg1, "v0") == 0)
        {
            // return value in v0 or f0/f2
            if (destFloat)
            {
                // We assume float returns are in $f0
                // Just mov.s from $f0 to var
                const char *destFReg = allocateFloatRegister();
                setFloatRegisterForVariable(current->result, destFReg);
                fprintf(outputFile, "\tmov.s %s, $f0\n", destFReg);
            }
            else
            {
                const char *destReg = allocateRegister();
                setRegisterForVariable(current->result, destReg);
                fprintf(outputFile, "\tmove %s, $v0\n", destReg);
            }
        }
        else
        {
            if (destFloat)
            {
                const char *fReg = ensureFloatInRegister(current->arg1, symTab);
                const char *destFReg = getFloatRegisterForVariable(current->result);
                if (!destFReg)
                {
                    destFReg = allocateFloatRegister();
                    setFloatRegisterForVariable(current->result, destFReg);
                }
                fprintf(outputFile, "\tmov.s %s, %s\n", destFReg, fReg);
            }
            else
            {
                const char *srcReg = ensureIntInRegister(current->arg1, symTab);
                const char *destReg = getRegisterForVariable(current->result);
                if (!destReg)
                {
                    destReg = allocateRegister();
                    setRegisterForVariable(current->result, destReg);
                }
                fprintf(outputFile, "\tmove %s, %s\n", destReg, srcReg);
            }
        }
    }
    else if (strcmp(current->op, "+") == 0 ||
             strcmp(current->op, "-") == 0 ||
             strcmp(current->op, "*") == 0 ||
             strcmp(current->op, "/") == 0)
    {
        bool leftFloat = isFloatTypeOperand(current->arg1, symTab);
        bool rightFloat = isFloatTypeOperand(current->arg2, symTab);
        bool resultFloat = leftFloat || rightFloat; // If any operand is float, result is float

        if (resultFloat)
        {
            fprintf(outputFile, "# Float arithmetic %s\n", current->op);
            const char *leftFReg = ensureFloatInRegister(current->arg1, symTab);
            const char *rightFReg = ensureFloatInRegister(current->arg2, symTab);
            const char *resFReg = allocateFloatRegister();
            setFloatRegisterForVariable(current->result, resFReg);

            if (strcmp(current->op, "+") == 0)
            {
                fprintf(outputFile, "\tadd.s %s, %s, %s\n", resFReg, leftFReg, rightFReg);
            }
            else if (strcmp(current->op, "-") == 0)
            {
                fprintf(outputFile, "\tsub.s %s, %s, %s\n", resFReg, leftFReg, rightFReg);
            }
            else if (strcmp(current->op, "*") == 0)
            {
                fprintf(outputFile, "\tmul.s %s, %s, %s\n", resFReg, leftFReg, rightFReg);
            }
            else if (strcmp(current->op, "/") == 0)
            {
                fprintf(outputFile, "\tdiv.s %s, %s, %s\n", resFReg, leftFReg, rightFReg);
            }
        }
        else
        {
            fprintf(outputFile, "# Integer arithmetic %s\n", current->op);
            const char *reg1 = ensureIntInRegister(current->arg1, symTab);
            const char *reg2 = ensureIntInRegister(current->arg2, symTab);
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
    }
    else if (strcmp(current->op, "write") == 0)
    {
        fprintf(outputFile, "# Write integer\n");
        const char *srcReg = ensureIntInRegister(current->arg1, symTab);
        fprintf(outputFile, "\tmove $a0, %s\n", srcReg);
        fprintf(outputFile, "\tli $v0, 1\n");
        fprintf(outputFile, "\tsyscall\n");
        fprintf(outputFile, "\tli $a0, 10\n");
        fprintf(outputFile, "\tli $v0, 11\n");
        fprintf(outputFile, "\tsyscall\n");
    }
    else if (strcmp(current->op, "write_float") == 0)
    {
        fprintf(outputFile, "# Write float\n");
        const char *fReg = ensureFloatInRegister(current->arg1, symTab);
        fprintf(outputFile, "\tmov.s $f12, %s\n", fReg);
        fprintf(outputFile, "\tli $v0, 2\n");
        fprintf(outputFile, "\tsyscall\n");
        fprintf(outputFile, "\tli $a0, 10\n");
        fprintf(outputFile, "\tli $v0, 11\n");
        fprintf(outputFile, "\tsyscall\n");
    }
    else if (strcmp(current->op, "[]=") == 0)
    {
        // Array store always int for simplicity. If needed float arrays, handle similarly.
        fprintf(outputFile, "# Array store\n");
        fprintf(outputFile, "\tla %s, %s\n", BASE_ADDRESS_REGISTER, current->arg1);

        char *offsetValue = computeOffset(current->arg2, 4);
        const char *valueReg = ensureIntInRegister(current->result, symTab);

        if (offsetValue != NULL)
        {
            fprintf(outputFile, "\tsw %s, %s(%s)\n", valueReg, offsetValue, BASE_ADDRESS_REGISTER);
            free(offsetValue);
        }
        else
        {
            const char *indexReg = ensureIntInRegister(current->arg2, symTab);
            fprintf(outputFile, "\tmul %s, %s, 4\n", ADDRESS_CALC_REGISTER, indexReg);
            fprintf(outputFile, "\tadd %s, %s, %s\n", ADDRESS_CALC_REGISTER, BASE_ADDRESS_REGISTER, ADDRESS_CALC_REGISTER);
            fprintf(outputFile, "\tsw %s, 0(%s)\n", valueReg, ADDRESS_CALC_REGISTER);
        }
    }
    else if (strcmp(current->op, "=[]") == 0)
    {
        fprintf(outputFile, "# Array load\n");
        fprintf(outputFile, "\tla %s, %s\n", BASE_ADDRESS_REGISTER, current->arg1);

        char *offsetValue = computeOffset(current->arg2, 4);
        const char *resultReg = allocateRegister();
        setRegisterForVariable(current->result, resultReg);

        if (offsetValue != NULL)
        {
            fprintf(outputFile, "\tlw %s, %s(%s)\n", resultReg, offsetValue, BASE_ADDRESS_REGISTER);
            free(offsetValue);
        }
        else
        {
            const char *indexReg = ensureIntInRegister(current->arg2, symTab);
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
        const char *condReg = ensureIntInRegister(current->arg1, symTab);
        fprintf(outputFile, "\tbeq %s, $zero, %s\n", condReg, current->result);
    }
    else if (strcmp(current->op, "goto") == 0)
    {
        fprintf(outputFile, "# Unconditional goto\n");
        fprintf(outputFile, "\tj %s\n", current->arg1);
    }
    else if (strcmp(current->op, "!") == 0)
    {
        fprintf(outputFile, "# Logical NOT\n");
        const char *exprReg = ensureIntInRegister(current->arg1, symTab);
        const char *resultReg = allocateRegister();
        setRegisterForVariable(current->result, resultReg);
        fprintf(outputFile, "\tseq %s, %s, $zero\n", resultReg, exprReg);
    }
    else if (strcmp(current->op, "&&") == 0 || strcmp(current->op, "||") == 0)
    {
        fprintf(outputFile, "# Logical operation %s\n", current->op);
        const char *leftReg = ensureIntInRegister(current->arg1, symTab);
        const char *rightReg = ensureIntInRegister(current->arg2, symTab);
        const char *resultReg = allocateRegister();
        setRegisterForVariable(current->result, resultReg);

        fprintf(outputFile, "\tsne %s, %s, $zero\n", leftReg, leftReg);
        fprintf(outputFile, "\tsne %s, %s, $zero\n", rightReg, rightReg);

        if (strcmp(current->op, "&&") == 0)
        {
            fprintf(outputFile, "\tand %s, %s, %s\n", resultReg, leftReg, rightReg);
        }
        else
        {
            fprintf(outputFile, "\tor %s, %s, %s\n", resultReg, leftReg, rightReg);
        }
    }
    else if (strcmp(current->op, "==") == 0 || strcmp(current->op, "!=") == 0 ||
             strcmp(current->op, "<") == 0 || strcmp(current->op, "<=") == 0 ||
             strcmp(current->op, ">") == 0 || strcmp(current->op, ">=") == 0)
    {
        fprintf(outputFile, "# Relational operation %s\n", current->op);
        const char *leftReg = ensureIntInRegister(current->arg1, symTab);
        const char *rightReg = ensureIntInRegister(current->arg2, symTab);
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
            const char *tempReg = allocateRegister();
            const char *tempReg2 = allocateRegister();
            fprintf(outputFile, "\tslt %s, %s, %s\n", tempReg, leftReg, rightReg);
            fprintf(outputFile, "\tseq %s, %s, %s\n", tempReg2, leftReg, rightReg);
            fprintf(outputFile, "\tor %s, %s, %s\n", resultReg, tempReg, tempReg2);
            deallocateRegister(tempReg);
            deallocateRegister(tempReg2);
        }
        else if (strcmp(current->op, ">") == 0)
        {
            // > is slt right,left
            fprintf(outputFile, "\tslt %s, %s, %s\n", resultReg, rightReg, leftReg);
        }
        else if (strcmp(current->op, ">=") == 0)
        {
            const char *tempReg = allocateRegister();
            fprintf(outputFile, "\tslt %s, %s, %s\n", tempReg, leftReg, rightReg);
            fprintf(outputFile, "\tseq %s, %s, $zero\n", resultReg, tempReg);
            deallocateRegister(tempReg);
        }
    }
    else
    {
        fprintf(stderr, "Warning: Unsupported TAC operation '%s'\n", current->op);
    }

    // Store variables back to memory if not used later
    const char *variablesToCheck[] = {current->arg1, current->arg2, current->result};
    for (int i = 0; i < 3; i++)
    {
        const char *var = variablesToCheck[i];
        if (var != NULL && isVariableInRegisterMap(var))
        {
            if (!isVariableUsedLater(current, var) && findSymbol(symTab, var))
            {
                // Store back to memory
                const char *regName = getRegisterForVariable(var);
                // var is int
                fprintf(outputFile, "\tsw %s, %s\n", regName, var);
                deallocateRegister(regName);
                removeVariableFromRegisterMap(var);
            }
        }

        if (var != NULL && isVariableInFloatRegisterMap(var))
        {
            if (!isVariableUsedLater(current, var) && findSymbol(symTab, var))
            {
                const char *regName = getFloatRegisterForVariable(var);
                // var is float
                fprintf(outputFile, "\ts.s %s, %s\n", regName, var);
                deallocateFloatRegister(regName);
                removeVariableFromFloatRegisterMap(var);
            }
        }
    }
}
