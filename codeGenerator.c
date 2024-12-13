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
static int floatConstCount = 0;

bool isTemporaryVariable(const char *operand)
{
    if (operand == NULL) return false;
    if (operand[0] != 't') return false;
    for (int i = 1; operand[i] != '\0'; i++)
    {
        if (!isdigit((unsigned char)operand[i]))
            return false;
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
            return;
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
            return;
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

int addFloatConstantToData(const char *value)
{
    fprintf(outputFile, "float_%d: .float %s\n", floatConstCount, value);
    return floatConstCount++;
}

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

// Helper to load a variable or label address into a register
static const char *loadAddress(const char *label)
{
    const char *tempReg = allocateRegister();
    fprintf(outputFile, "\tla %s, %s\n", tempReg, label);
    return tempReg;
}

// Load float from memory using lwc1 instead of l.s
static void loadFloatFromMemory(const char *freg, const char *label, const char *offset)
{
    const char *tempReg = allocateRegister();
    fprintf(outputFile, "\tla %s, %s\n", tempReg, label);
    if (offset)
        fprintf(outputFile, "\tlwc1 %s, %s(%s)\n", freg, offset, tempReg);
    else
        fprintf(outputFile, "\tlwc1 %s, 0(%s)\n", freg, tempReg);
    deallocateRegister(tempReg);
}

// Load int from memory remains the same
static void loadIntFromMemory(const char *reg, const char *label, const char *offset)
{
    const char *tempReg = allocateRegister();
    fprintf(outputFile, "\tla %s, %s\n", tempReg, label);
    if (offset)
        fprintf(outputFile, "\tlw %s, %s(%s)\n", reg, offset, tempReg);
    else
        fprintf(outputFile, "\tlw %s, 0(%s)\n", reg, tempReg);
    deallocateRegister(tempReg);
}

const char *ensureFloatInRegister(const char *operand, SymbolTable *symTab)
{
    // If operand is already in a float register
    if (isVariableInFloatRegisterMap(operand))
    {
        return getFloatRegisterForVariable(operand);
    }
    // If operand is a float constant
    else if (isFloatConstant(operand))
    {
        const char *fReg = allocateFloatRegister();
        int constIndex = addFloatConstantToData(operand);
        const char *tempReg = allocateRegister();
        fprintf(outputFile, "\tla %s, float_%d\n", tempReg, constIndex);
        fprintf(outputFile, "\tlwc1 %s, 0(%s)\n", fReg, tempReg);
        deallocateRegister(tempReg);

        setFloatRegisterForVariable(operand, fReg);
        return fReg;
    }
    // If operand is an integer constant that needs to be treated as float
    else if (isConstant(operand) && !isFloatConstant(operand))
    {
        const char *intReg = allocateRegister();
        loadIntegerConstant(intReg, operand);
        const char *fReg = allocateFloatRegister();
        fprintf(outputFile, "\tmtc1 %s, %s\n", intReg, fReg);
        fprintf(outputFile, "\tcvt.s.w %s, %s\n", fReg, fReg);
        setFloatRegisterForVariable(operand, fReg);
        deallocateRegister(intReg);
        return fReg;
    }
    else if (isVariableInRegisterMap(operand))
    {
        // Operand is an int in a register, convert it to float
        const char *intReg = getRegisterForVariable(operand);
        const char *fReg = allocateFloatRegister();
        fprintf(outputFile, "\tmtc1 %s, %s\n", intReg, fReg);
        fprintf(outputFile, "\tcvt.s.w %s, %s\n", fReg, fReg);
        setFloatRegisterForVariable(operand, fReg);
        return fReg;
    }
    else
    {
        // Load from memory as a float variable
        Symbol *sym = findSymbol(symTab, operand);
        if (!sym && isTemporaryVariable(operand))
        {
            fprintf(stderr, "Error: Attempting to load temporary variable '%s' from memory.\n", operand);
            exit(1);
        }
        
        const char *fReg = allocateFloatRegister();
        const char *tempReg = allocateRegister();
        fprintf(outputFile, "\tla %s, %s\n", tempReg, operand);
        fprintf(outputFile, "\tlwc1 %s, 0(%s)\n", fReg, tempReg);
        deallocateRegister(tempReg);

        setFloatRegisterForVariable(operand, fReg);
        return fReg;
    }
}

const char *ensureIntInRegister(const char *operand, SymbolTable *symTab)
{
    if (isVariableInRegisterMap(operand))
    {
        return getRegisterForVariable(operand);
    }
    else if (isConstant(operand) && !isFloatConstant(operand))
    {
        const char *reg = allocateRegister();
        loadIntegerConstant(reg, operand);
        setRegisterForVariable(operand, reg);
        return reg;
    }
    else if (isFloatConstant(operand))
    {
        const char *fReg = allocateFloatRegister();
        int constIndex = addFloatConstantToData(operand);
        const char *tempReg = allocateRegister();
        fprintf(outputFile, "\tla %s, float_%d\n", tempReg, constIndex);
        fprintf(outputFile, "\tlwc1 %s, 0(%s)\n", fReg, tempReg);
        deallocateRegister(tempReg);

        fprintf(outputFile, "\tcvt.w.s %s, %s\n", fReg, fReg);
        const char *intReg = allocateRegister();
        fprintf(outputFile, "\tmfc1 %s, %s\n", intReg, fReg);
        deallocateFloatRegister(fReg);

        setRegisterForVariable(operand, intReg);
        return intReg;
    }
    else if (isVariableInFloatRegisterMap(operand))
    {
        const char *fReg = getFloatRegisterForVariable(operand);
        fprintf(outputFile, "\tcvt.w.s %s, %s\n", fReg, fReg);
        const char *intReg = allocateRegister();
        fprintf(outputFile, "\tmfc1 %s, %s\n", intReg, fReg);
        setRegisterForVariable(operand, intReg);
        return intReg;
    }
    else
    {
        Symbol *sym = findSymbol(symTab, operand);
        if (!sym && isTemporaryVariable(operand))
        {
            fprintf(stderr, "Error: Attempting to load temporary variable '%s' from memory.\n", operand);
            exit(1);
        }

        const char *reg = allocateRegister();
        const char *tempReg = allocateRegister();
        fprintf(outputFile, "\tla %s, %s\n", tempReg, operand);
        fprintf(outputFile, "\tlw %s, 0(%s)\n", reg, tempReg);
        deallocateRegister(tempReg);

        setRegisterForVariable(operand, reg);
        return reg;
    }
}

void handleFunctionArguments(TAC *current, int *argCount)
{
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

    // Float arguments simplified assumption (handled similarly to ints)
    if (current->arg2 && strcmp(current->arg2, "float") == 0)
    {
        const char *fReg = ensureFloatInRegister(current->arg1, NULL);
        if (*argCount == 0)
        {
            fprintf(outputFile, "\tmov.s $f12, %s\n", fReg);
        }
        else
        {
            fprintf(stderr, "Warning: Only first float argument supported directly\n");
        }
    }
    else
    {
        const char *srcReg = ensureIntInRegister(current->arg1, NULL);
        fprintf(outputFile, "\tmove %s, %s\n", argReg, srcReg);
    }

    (*argCount)++;
}

void storeIfNotUsedLater(const char *var, TAC *current, SymbolTable *symTab)
{
    if (var == NULL) return;
    Symbol *sym = findSymbol(symTab, var);
    bool isTemp = isTemporaryVariable(var);
    if (!isTemp && sym != NULL) {
        // Real variable
        if (!isVariableUsedLater(current, var))
        {
            if (isVariableInRegisterMap(var))
            {
                const char *regName = getRegisterForVariable(var);
                const char *tempReg = allocateRegister();
                fprintf(outputFile, "\tla %s, %s\n", tempReg, var);
                fprintf(outputFile, "\tsw %s, 0(%s)\n", regName, tempReg);
                deallocateRegister(tempReg);

                deallocateRegister(regName);
                removeVariableFromRegisterMap(var);
            }
            if (isVariableInFloatRegisterMap(var))
            {
                const char *regName = getFloatRegisterForVariable(var);
                const char *tempReg = allocateRegister();
                fprintf(outputFile, "\tla %s, %s\n", tempReg, var);
                fprintf(outputFile, "\tswc1 %s, 0(%s)\n", regName, tempReg);
                deallocateRegister(tempReg);

                deallocateFloatRegister(regName);
                removeVariableFromFloatRegisterMap(var);
            }
        }
    }
    else
    {
        // It's a temporary or not in symbol table
        // If not used later, just free register
        if (!isVariableUsedLater(current, var))
        {
            if (isVariableInRegisterMap(var))
            {
                const char *regName = getRegisterForVariable(var);
                deallocateRegister(regName);
                removeVariableFromRegisterMap(var);
            }
            if (isVariableInFloatRegisterMap(var))
            {
                const char *freg = getFloatRegisterForVariable(var);
                deallocateFloatRegister(freg);
                removeVariableFromFloatRegisterMap(var);
            }
        }
    }
}

void generateTACOperation(TAC *current, SymbolTable *symTab, const char *currentFunctionName)
{
    if (!current->op) return;

    // Float arithmetic operations
    if (strcmp(current->op, "fadd") == 0 ||
        strcmp(current->op, "fsub") == 0 ||
        strcmp(current->op, "fmul") == 0 ||
        strcmp(current->op, "fdiv") == 0)
    {
        const char *reg1 = ensureFloatInRegister(current->arg1, symTab);
        const char *reg2 = ensureFloatInRegister(current->arg2, symTab);
        const char *resultReg = allocateFloatRegister();
        setFloatRegisterForVariable(current->result, resultReg);

        if (strcmp(current->op, "fadd") == 0)
            fprintf(outputFile, "\tadd.s %s, %s, %s\n", resultReg, reg1, reg2);
        else if (strcmp(current->op, "fsub") == 0)
            fprintf(outputFile, "\tsub.s %s, %s, %s\n", resultReg, reg1, reg2);
        else if (strcmp(current->op, "fmul") == 0)
            fprintf(outputFile, "\tmul.s %s, %s, %s\n", resultReg, reg1, reg2);
        else if (strcmp(current->op, "fdiv") == 0)
            fprintf(outputFile, "\tdiv.s %s, %s, %s\n", resultReg, reg1, reg2);

        storeIfNotUsedLater(current->arg1, current, symTab);
        storeIfNotUsedLater(current->arg2, current, symTab);
        storeIfNotUsedLater(current->result, current, symTab);
        return;
    }

    // Integer arithmetic operations
    if (strcmp(current->op, "+") == 0 || strcmp(current->op, "-") == 0 ||
        strcmp(current->op, "*") == 0 || strcmp(current->op, "/") == 0)
    {
        const char *reg1 = ensureIntInRegister(current->arg1, symTab);
        const char *reg2 = ensureIntInRegister(current->arg2, symTab);
        const char *resultReg = allocateRegister();
        setRegisterForVariable(current->result, resultReg);

        if (strcmp(current->op, "+") == 0)
            fprintf(outputFile, "\tadd %s, %s, %s\n", resultReg, reg1, reg2);
        else if (strcmp(current->op, "-") == 0)
            fprintf(outputFile, "\tsub %s, %s, %s\n", resultReg, reg1, reg2);
        else if (strcmp(current->op, "*") == 0)
            fprintf(outputFile, "\tmul %s, %s, %s\n", resultReg, reg1, reg2);
        else if (strcmp(current->op, "/") == 0) {
            fprintf(outputFile, "\tdiv %s, %s\n", reg1, reg2);
            fprintf(outputFile, "\tmflo %s\n", resultReg);
        }

        storeIfNotUsedLater(current->arg1, current, symTab);
        storeIfNotUsedLater(current->arg2, current, symTab);
        storeIfNotUsedLater(current->result, current, symTab);
        return;
    }

    // Assignment =
    if (strcmp(current->op, "=") == 0)
    {
        bool destFloat = isFloatVariable(current->result, symTab);
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

        storeIfNotUsedLater(current->arg1, current, symTab);
        storeIfNotUsedLater(current->result, current, symTab);
        return;
    }

    // Write int
    if (strcmp(current->op, "write") == 0)
    {
        const char *srcReg = ensureIntInRegister(current->arg1, symTab);
        fprintf(outputFile, "\tmove $a0, %s\n", srcReg);
        fprintf(outputFile, "\tli $v0, 1\n");
        fprintf(outputFile, "\tsyscall\n");
        // Newline
        fprintf(outputFile, "\tli $a0, 10\n");
        fprintf(outputFile, "\tli $v0, 11\n");
        fprintf(outputFile, "\tsyscall\n");

        storeIfNotUsedLater(current->arg1, current, symTab);
        return;
    }

    // Write float
    if (strcmp(current->op, "write_float") == 0)
    {
        const char *fReg = ensureFloatInRegister(current->arg1, symTab);
        fprintf(outputFile, "\tmov.s $f12, %s\n", fReg);
        fprintf(outputFile, "\tli $v0, 2\n");
        fprintf(outputFile, "\tsyscall\n");
        // Newline
        fprintf(outputFile, "\tli $a0, 10\n");
        fprintf(outputFile, "\tli $v0, 11\n");
        fprintf(outputFile, "\tsyscall\n");

        storeIfNotUsedLater(current->arg1, current, symTab);
        return;
    }

    // Array store []=
    if (strcmp(current->op, "[]=") == 0)
    {
        const char *arrayName = current->arg1;
        const char *indexOperand = current->arg2;
        const char *valueOperand = current->result;

        fprintf(outputFile, "\tla %s, %s\n", BASE_ADDRESS_REGISTER, arrayName);
        char *offsetValue = computeOffset(indexOperand, 4);
        const char *valueReg = ensureIntInRegister(valueOperand, symTab);

        if (offsetValue != NULL)
        {
            fprintf(outputFile, "\tsw %s, %s(%s)\n", valueReg, offsetValue, BASE_ADDRESS_REGISTER);
            free(offsetValue);
        }
        else
        {
            const char *indexReg = ensureIntInRegister(indexOperand, symTab);
            fprintf(outputFile, "\tmul %s, %s, 4\n", ADDRESS_CALC_REGISTER, indexReg);
            fprintf(outputFile, "\tadd %s, %s, %s\n", ADDRESS_CALC_REGISTER, BASE_ADDRESS_REGISTER, ADDRESS_CALC_REGISTER);
            fprintf(outputFile, "\tsw %s, 0(%s)\n", valueReg, ADDRESS_CALC_REGISTER);
            storeIfNotUsedLater(indexOperand, current, symTab);
        }

        storeIfNotUsedLater(valueOperand, current, symTab);
        storeIfNotUsedLater(arrayName, current, symTab);
        return;
    }

    // Array load =[]
    if (strcmp(current->op, "=[]") == 0)
    {
        const char *arrayName = current->arg1;
        const char *indexOperand = current->arg2;
        const char *destVar = current->result;

        fprintf(outputFile, "\tla %s, %s\n", BASE_ADDRESS_REGISTER, arrayName);
        char *offsetValue = computeOffset(indexOperand, 4);
        const char *resultReg = allocateRegister();
        setRegisterForVariable(destVar, resultReg);

        if (offsetValue != NULL)
        {
            fprintf(outputFile, "\tlw %s, %s(%s)\n", resultReg, offsetValue, BASE_ADDRESS_REGISTER);
            free(offsetValue);
        }
        else
        {
            const char *indexReg = ensureIntInRegister(indexOperand, symTab);
            fprintf(outputFile, "\tmul %s, %s, 4\n", ADDRESS_CALC_REGISTER, indexReg);
            fprintf(outputFile, "\tadd %s, %s, %s\n", ADDRESS_CALC_REGISTER, BASE_ADDRESS_REGISTER, ADDRESS_CALC_REGISTER);
            fprintf(outputFile, "\tlw %s, 0(%s)\n", resultReg, ADDRESS_CALC_REGISTER);
            storeIfNotUsedLater(indexOperand, current, symTab);
        }

        storeIfNotUsedLater(arrayName, current, symTab);
        storeIfNotUsedLater(destVar, current, symTab);
        return;
    }

    // param
    if (strcmp(current->op, "param") == 0)
    {
        handleFunctionArguments(current, &currentArgCount);
        return;
    }

    // call
    if (strcmp(current->op, "call") == 0)
    {
        fprintf(outputFile, "\tjal %s\n", current->arg1);
        currentArgCount = 0; // Reset arg count
        return;
    }

    // ifFalse
    if (strcmp(current->op, "ifFalse") == 0)
    {
        const char *condReg = ensureIntInRegister(current->arg1, symTab);
        fprintf(outputFile, "\tbeq %s, $zero, %s\n", condReg, current->result);
        storeIfNotUsedLater(current->arg1, current, symTab);
        return;
    }

    // goto
    if (strcmp(current->op, "goto") == 0)
    {
        fprintf(outputFile, "\tj %s\n", current->arg1);
        return;
    }

    // NOT !
    if (strcmp(current->op, "!") == 0)
    {
        const char *exprReg = ensureIntInRegister(current->arg1, symTab);
        const char *resultReg = allocateRegister();
        setRegisterForVariable(current->result, resultReg);
        fprintf(outputFile, "\tseq %s, %s, $zero\n", resultReg, exprReg);
        storeIfNotUsedLater(current->arg1, current, symTab);
        storeIfNotUsedLater(current->result, current, symTab);
        return;
    }

    // Logical && ||
    if (strcmp(current->op, "&&") == 0 || strcmp(current->op, "||") == 0)
    {
        const char *leftReg = ensureIntInRegister(current->arg1, symTab);
        const char *rightReg = ensureIntInRegister(current->arg2, symTab);
        const char *resultReg = allocateRegister();
        setRegisterForVariable(current->result, resultReg);

        fprintf(outputFile, "\tsne %s, %s, $zero\n", leftReg, leftReg);
        fprintf(outputFile, "\tsne %s, %s, $zero\n", rightReg, rightReg);

        if (strcmp(current->op, "&&") == 0)
            fprintf(outputFile, "\tand %s, %s, %s\n", resultReg, leftReg, rightReg);
        else
            fprintf(outputFile, "\tor %s, %s, %s\n", resultReg, leftReg, rightReg);

        storeIfNotUsedLater(current->arg1, current, symTab);
        storeIfNotUsedLater(current->arg2, current, symTab);
        storeIfNotUsedLater(current->result, current, symTab);
        return;
    }

    // Relational ops ==, !=, <, <=, >, >=
    if (strcmp(current->op, "==") == 0 || strcmp(current->op, "!=") == 0 ||
        strcmp(current->op, "<") == 0 || strcmp(current->op, "<=") == 0 ||
        strcmp(current->op, ">") == 0 || strcmp(current->op, ">=") == 0)
    {
        const char *leftReg = ensureIntInRegister(current->arg1, symTab);
        const char *rightReg = ensureIntInRegister(current->arg2, symTab);
        const char *resultReg = allocateRegister();
        setRegisterForVariable(current->result, resultReg);

        if (strcmp(current->op, "==") == 0)
            fprintf(outputFile, "\tseq %s, %s, %s\n", resultReg, leftReg, rightReg);
        else if (strcmp(current->op, "!=") == 0)
            fprintf(outputFile, "\tsne %s, %s, %s\n", resultReg, leftReg, rightReg);
        else if (strcmp(current->op, "<") == 0)
            fprintf(outputFile, "\tslt %s, %s, %s\n", resultReg, leftReg, rightReg);
        else if (strcmp(current->op, "<=") == 0) {
            const char *tempReg = allocateRegister();
            const char *tempReg2 = allocateRegister();
            fprintf(outputFile, "\tslt %s, %s, %s\n", tempReg, leftReg, rightReg);
            fprintf(outputFile, "\tseq %s, %s, %s\n", tempReg2, leftReg, rightReg);
            fprintf(outputFile, "\tor %s, %s, %s\n", resultReg, tempReg, tempReg2);
            deallocateRegister(tempReg);
            deallocateRegister(tempReg2);
        }
        else if (strcmp(current->op, ">") == 0)
            fprintf(outputFile, "\tslt %s, %s, %s\n", resultReg, rightReg, leftReg);
        else if (strcmp(current->op, ">=") == 0) {
            const char *tempReg = allocateRegister();
            fprintf(outputFile, "\tslt %s, %s, %s\n", tempReg, leftReg, rightReg);
            fprintf(outputFile, "\tseq %s, %s, $zero\n", resultReg, tempReg);
            deallocateRegister(tempReg);
        }

        storeIfNotUsedLater(current->arg1, current, symTab);
        storeIfNotUsedLater(current->arg2, current, symTab);
        storeIfNotUsedLater(current->result, current, symTab);
        return;
    }

    // Unsupported operation
    fprintf(stderr, "Warning: Unsupported TAC operation '%s'\n", current->op);
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
                    if(strcmp(current->result, "NULL") != 0)
                    {
                        fprintf(outputFile, "\tli $v0, %s\n", current->result ? current->result : "NULL");
                    }
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
                fprintf(outputFile, "%s: .float %s\n", symbol->name, symbol->value);
            else
                fprintf(outputFile, "%s: .float 0.0\n", symbol->name);
        }
        else if (strcmp(symbol->type, "int") == 0)
        {
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

    // Separate main and other functions
    while (current != NULL)
    {
        if (current->op && strcmp(current->op, "label") == 0)
        {
            currentFunctionName = current->arg1;
            inFunction = true;

            if (strcmp(currentFunctionName, "main") == 0)
                inMainFunction = true;
            else
                inMainFunction = false;
        }

        if (inFunction)
        {
            if (inMainFunction)
            {
                if (!mainTAC) mainTAC = mainTACTail = current;
                else {
                    mainTACTail->next = current;
                    mainTACTail = current;
                }
            }
            else
            {
                if (!otherFunctionsTAC) otherFunctionsTAC = otherFunctionsTACTail = current;
                else {
                    otherFunctionsTACTail->next = current;
                    otherFunctionsTACTail = current;
                }
            }
        }

        current = current->next;
    }

    if (mainTACTail != NULL)
        mainTACTail->next = NULL;
    if (otherFunctionsTACTail != NULL)
        otherFunctionsTACTail->next = NULL;

    // Process main
    processTACList(mainTAC, symTab);
    fprintf(outputFile, "\n");
    // Process other functions
    processTACList(otherFunctionsTAC, symTab);
}
