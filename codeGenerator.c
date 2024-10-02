// codeGenerator.c

#include "codeGenerator.h"
#include "optimizer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static FILE *outputFile;

typedef struct
{
    char *name; // Name of the register, e.g., "$t0"
    bool inUse; // Whether the register is currently in use
} MIPSRegister;

// Array of temporary registers, used for register allocation
// and tracking which registers are currently in use

MIPSRegister tempRegisters[NUM_TEMP_REGISTERS] = {
    {"$t0", false}, {"$t1", false}, {"$t2", false}, {"$t3", false}, {"$t4", false}, {"$t5", false}, {"$t6", false}, {"$t7", false}, {"$t8", false}, {"$t9", false}};

void initCodeGenerator(const char *outputFilename)
{
    outputFile = fopen(outputFilename, "w");
    if (outputFile == NULL)
    {
        perror("Failed to open output file");
        exit(EXIT_FAILURE);
    }
}

void generateMIPS(TAC *tacInstructions)
{
    /*
    NOTE: This is a simple code generation function that generates MIPS assembly code
    from the provided TAC instructions. It assumes that the TAC is already optimized

    The generated code is written to the output file specified during initialization.
    The register allocation strategy used here is a simple linear scan of the temporary
    registers, without considering more advanced techniques such as liveness analysis.

    You can modify this function to implement more advanced register allocation
    and other optimizations as needed for your compiler.

    Not all generated MIPS uses the allocated registers, so you may need to modify
    the code to properly allocate registers for each operation.

    */

    VarNode *varList = NULL; // Declare and initialize varList

    // Collect variables from TAC instructions
    collectVariables(tacInstructions, &varList);

    // Generate the .data section
    fprintf(outputFile, ".data\n");
    VarNode *varCurrent = varList;
    while (varCurrent != NULL) {
        if (varCurrent->isInitialized) {
            fprintf(outputFile, "%s: .word %d\n", varCurrent->name, varCurrent->initialValue);
        } else {
            fprintf(outputFile, "%s: .word 0\n", varCurrent->name);
        }
        varCurrent = varCurrent->next;
    }

    // Start the .text section and main function
    fprintf(outputFile, ".text\n");
    fprintf(outputFile, ".globl main\n");
    fprintf(outputFile, "main:\n");

    TAC *current = tacInstructions;
    while (current != NULL) {
        if (current->op != NULL) {
            if (strcmp(current->op, "+") == 0 || strcmp(current->op, "-") == 0 ||
                strcmp(current->op, "*") == 0 || strcmp(current->op, "/") == 0) {
                // Generate code for binary operations
                fprintf(outputFile, "# Generating MIPS code for operation %s\n", current->op);
                // Load operands
                loadOperand(current->arg1, "$t0");
                loadOperand(current->arg2, "$t1");
                // Perform operation
                if (strcmp(current->op, "+") == 0) {
                    fprintf(outputFile, "\tadd $t2, $t0, $t1\n");
                } else if (strcmp(current->op, "-") == 0) {
                    fprintf(outputFile, "\tsub $t2, $t0, $t1\n");
                } else if (strcmp(current->op, "*") == 0) {
                    fprintf(outputFile, "\tmul $t2, $t0, $t1\n");
                } else if (strcmp(current->op, "/") == 0) {
                    fprintf(outputFile, "\tdiv $t0, $t1\n");
                    fprintf(outputFile, "\tmflo $t2\n");
                }
                // Store result
                fprintf(outputFile, "\tsw $t2, %s\n", current->result);
            } else if (strcmp(current->op, "=") == 0) {
                // Assignment operation
                fprintf(outputFile, "# Generating MIPS code for assignment\n");
                loadOperand(current->arg1, "$t0");
                fprintf(outputFile, "\tsw $t0, %s\n", current->result);
            } else if (strcmp(current->op, "write") == 0) {
                // Write operation
                fprintf(outputFile, "# Generating MIPS code for write operation\n");
                loadOperand(current->arg1, "$a0");
                fprintf(outputFile, "\tli $v0, 1\n");      // Syscall code for print_int
                fprintf(outputFile, "\tsyscall\n");
                // Print newline character
                fprintf(outputFile, "\tli $a0, 10\n");     // ASCII code for newline
                fprintf(outputFile, "\tli $v0, 11\n");     // Syscall code for print_char
                fprintf(outputFile, "\tsyscall\n");
            } else {
                fprintf(stderr, "Warning: Unsupported TAC operation '%s'\n", current->op);
            }
        }
        current = current->next;
    }

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

/*

Implementing register allocation in your MIPS code generation phase is a crucial step
for optimizing the use of CPU resources. The goal is to map your temporary variables (from TAC)
to a limited set of MIPS registers, ideally minimizing memory access by keeping
frequently used values in registers.

MIPS architecture provides a set of general-purpose registers,
$t0 to $t9 for temporary values, and $s0 to $s7 for saved values.
For simplicity, let's focus on using the temporary registers.

Strategy for Register Allocation:
--------------------------------

A simple strategy for register allocation could involve:

Register Pool: Keep track of which registers are currently available.
Allocation and Deallocation: Allocate registers when needed for operations and
                             deallocate them when they are no longer needed.
Spilling: If all registers are in use and another one is needed,
          "spill" a register's value to memory and reuse the register.

*/

// Allocate a register
int allocateRegister()
{
    for (int i = 0; i < NUM_TEMP_REGISTERS; i++)
    {
        if (!tempRegisters[i].inUse)
        {
            tempRegisters[i].inUse = true;
            return i; // Return the register index
        }
    }
    // No available register, implement spilling if necessary
    return -1; // Indicate failure
}

// Deallocate a register
void deallocateRegister(int regIndex)
{
    if (regIndex >= 0 && regIndex < NUM_TEMP_REGISTERS)
    {
        tempRegisters[regIndex].inUse = false;
    }
}

void printCurrentTAC(TAC *tac)
{
    TAC *current = tac;
    while (current != NULL)
    {
        printf("\n--- CURRENT TAC %s ---\n", current->op);
        if (strcmp(current->op, "=") == 0)
        {
            printf("%s = %s\n", current->result, current->arg1);
        }
        else
        {
            if (current->result != NULL)
                printf("%s = ", current->result);
            if (current->arg1 != NULL)
                printf("%s ", current->arg1);
            if (current->op != NULL)
                printf("%s ", current->op);
            if (current->arg2 != NULL)
                printf("%s ", current->arg2);
            printf("\n");
        }
        current = current->next;
    }
}

// Function to check if a variable is already in the list
bool isVariableInList(VarNode* varList, const char* varName) {
    VarNode* current = varList;
    while (current != NULL) {
        if (strcmp(current->name, varName) == 0) {
            return true;
        }
        current = current->next;
    }
    return false;
}

// Function to add a variable to the list
void addVariable(VarNode** varList, const char* varName, int initialValue, bool isInitialized) {
    VarNode* existingNode = *varList;
    while (existingNode != NULL) {
        if (strcmp(existingNode->name, varName) == 0) {
            // Update initial value if variable already exists and is uninitialized
            if (!existingNode->isInitialized && isInitialized) {
                existingNode->initialValue = initialValue;
                existingNode->isInitialized = true;
            }
            return; // Variable already in the list
        }
        existingNode = existingNode->next;
    }
    // Variable not found, create a new one
    VarNode* newNode = malloc(sizeof(VarNode));
    newNode->name = strdup(varName);
    newNode->initialValue = initialValue;
    newNode->isInitialized = isInitialized;
    newNode->next = *varList;
    *varList = newNode;
}


// Function to collect all variables from TAC instructions
void collectVariables(TAC* tacInstructions, VarNode** varList) {
    TAC* current = tacInstructions;
    while (current != NULL) {
        if (current->result != NULL && isVariable(current->result)) {
            int initialValue = 0;
            bool isInitialized = false;
            if (strcmp(current->op, "=") == 0 && isConstant(current->arg1)) {
                // Assignment of a constant to a variable (e.g., a = 5)
                initialValue = atoi(current->arg1);
                isInitialized = true;
                printf("Variable '%s' initialized to %d\n", current->result, initialValue);
            }
            addVariable(varList, current->result, initialValue, isInitialized);
        }
        if (current->arg1 != NULL && isVariable(current->arg1)) {
            addVariable(varList, current->arg1, 0, false);
        }
        if (current->arg2 != NULL && isVariable(current->arg2)) {
            addVariable(varList, current->arg2, 0, false);
        }
        current = current->next;
    }
}

// Function to free the variable list
void freeVariableList(VarNode* varList) {
    VarNode* current = varList;
    while (current != NULL) {
        VarNode* next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
}

VarNode* findVariable(VarNode* varList, const char* varName) {
    VarNode* current = varList;
    while (current != NULL) {
        if (strcmp(current->name, varName) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void loadOperand(const char *operand, const char *registerName) {
    if (isConstant(operand)) {
        fprintf(outputFile, "\tli %s, %s\n", registerName, operand);
    } else {
        fprintf(outputFile, "\tlw %s, %s\n", registerName, operand);
    }
}
