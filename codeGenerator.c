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
    // Write the data section header
    fprintf(outputFile, ".data\n");
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

    TAC *current = tacInstructions;
    fprintf(outputFile, ".text\n.globl main\nmain:\n");

    printf("Generating MIPS code...\n");
    while (current != NULL)
    {
        if (current->op == NULL) {
            fprintf(stderr, "Error: TAC operation is NULL\n");
            current = current->next;
            continue;
        }

        if (strcmp(current->op, "=") == 0)
        {
            if (current->result == NULL || current->arg1 == NULL) {
                fprintf(stderr, "Error: Assignment TAC contains NULL fields\n");
                current = current->next;
                continue;
            }

            int reg = allocateRegister();
            if (reg == -1)
            {
                fprintf(stderr, "Error: No available register for assignment operation\n");
                return;
            }

            if (isConstant(current->arg1))
            {
                // Load immediate value for constant assignment
                fprintf(outputFile, "\tli %s, %s\n", tempRegisters[reg].name, current->arg1);
            }
            else
            {
                // Load value from a variable
                fprintf(outputFile, "\tlw %s, %s\n", tempRegisters[reg].name, current->arg1);
            }

            fprintf(outputFile, "\tsw %s, %s\n", tempRegisters[reg].name, current->result);
            deallocateRegister(reg);
        }
        else if (strcmp(current->op, "+") == 0)
        {
            int reg1 = allocateRegister();
            int reg2 = allocateRegister();
            int resultReg = allocateRegister();
            if (current->arg1 == NULL || current->arg2 == NULL || current->result == NULL) {
                fprintf(stderr, "Error: Addition TAC contains NULL fields\n");
                current = current->next;
                continue;
            }
            printf("Generating MIPS code for addition operation\n");
            fprintf(outputFile, "\tlw %s, %s\n", tempRegisters[reg1].name, current->arg1);
            fprintf(outputFile, "\tlw %s, %s\n", tempRegisters[reg2].name, current->arg2);
            fprintf(outputFile, "\tadd %s, %s, %s\n", tempRegisters[resultReg].name, tempRegisters[reg1].name, tempRegisters[reg2].name);
            fprintf(outputFile, "\tsw %s, %s\n", tempRegisters[resultReg].name, current->result);

            // Deallocate the registers after use
            deallocateRegister(reg1);
            deallocateRegister(reg2);
            deallocateRegister(resultReg);
        }
        else if (strcmp(current->op, "-") == 0)
        {
            int reg1 = allocateRegister();
            int reg2 = allocateRegister();
            int resultReg = allocateRegister();

            if (reg1 == -1 || reg2 == -1 || resultReg == -1)
            {
                fprintf(stderr, "Error: No available registers for subtraction operation\n");
                return;
            }

            printf("Generating MIPS code for subtraction operation\n");
            fprintf(outputFile, "\tlw %s, %s\n", tempRegisters[reg1].name, current->arg1);
            fprintf(outputFile, "\tlw %s, %s\n", tempRegisters[reg2].name, current->arg2);
            fprintf(outputFile, "\tsub %s, %s, %s\n", tempRegisters[resultReg].name, tempRegisters[reg1].name, tempRegisters[reg2].name);
            fprintf(outputFile, "\tsw %s, %s\n", tempRegisters[resultReg].name, current->result);

            // Deallocate the registers after use
            deallocateRegister(reg1);
            deallocateRegister(reg2);
            deallocateRegister(resultReg);
        }
        else if (strcmp(current->op, "*") == 0)
        {
            int reg1 = allocateRegister();
            int reg2 = allocateRegister();
            int resultReg = allocateRegister();

            if (reg1 == -1 || reg2 == -1 || resultReg == -1)
            {
                fprintf(stderr, "Error: No available registers for multiplication operation\n");
                return;
            }

            printf("Generating MIPS code for multiplication operation\n");
            fprintf(outputFile, "\tlw %s, %s\n", tempRegisters[reg1].name, current->arg1);
            fprintf(outputFile, "\tlw %s, %s\n", tempRegisters[reg2].name, current->arg2);
            fprintf(outputFile, "\tmul %s, %s, %s\n", tempRegisters[resultReg].name, tempRegisters[reg1].name, tempRegisters[reg2].name);
            fprintf(outputFile, "\tsw %s, %s\n", tempRegisters[resultReg].name, current->result);

            // Deallocate the registers after use
            deallocateRegister(reg1);
            deallocateRegister(reg2);
            deallocateRegister(resultReg);
        }
        else if (strcmp(current->op, "write") == 0)
        {
            int reg = allocateRegister();
            if (reg == -1)
            {
                fprintf(stderr, "Error: No available register for write operation\n");
                return;
            }

            printf("Generating MIPS code for WRITE operation\n");

            // Check if arg1 is a constant (if it is a number)
            if (isConstant(current->arg1))
            {
                // Handle the case where arg1 is a constant value
                fprintf(outputFile, "\tli %s, %s\n", tempRegisters[reg].name, current->arg1); // Load immediate constant into the register
            }
            else
            {
                // Handle the case where arg1 is a variable (load it from memory)
                fprintf(outputFile, "\tlw %s, %s\n", tempRegisters[reg].name, current->arg1); // Load from memory into the register
            }

            // Move the value into $a0 for printing
            fprintf(outputFile, "\tmove $a0, %s\n", tempRegisters[reg].name);  // Load the value into $a0

            // MIPS syscall code for printing an integer
            fprintf(outputFile, "\tli $v0, 1\n");  // Syscall code 1 for print integer
            fprintf(outputFile, "\tsyscall\n");    // Perform the syscall

            // Deallocate the register after use
            deallocateRegister(reg);
        }

        current = current->next;
    }

    // Exit program
    fprintf(outputFile, "\tli $v0, 10\n\tsyscall\n");
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
