#include "optimizer.h"
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

void optimizeTAC(TAC **head)
{
    printf(" 1 ");
    int changes;
    do
    {
        changes = 0;
        changes += constantFolding(head);
        changes += constantPropagation(head);
        changes += copyPropagation(head);
        changes += deadCodeElimination(head);
    } while (changes > 0);
}

bool isConstant(const char *str)
{
    printf(" 2 ");
    if (str == NULL || *str == '\0')
    {
        return false;
    }

    // Handle negative numbers
    if (*str == '-')
    {
        ++str;
    }

    while (*str)
    {
        if (!isdigit((unsigned char)*str))
        {
            return false;
        }
        ++str;
    }

    return true;
}

bool isVariable(const char *str)
{
    printf(" 3 ");
    if (str == NULL || *str == '\0')
    {
        return false;
    }

    // Check if the first character is a letter or underscore
    if (!isalpha((unsigned char)*str) && *str != '_')
    {
        return false;
    }

    ++str;
    while (*str)
    {
        if (!isalnum((unsigned char)*str) && *str != '_')
        {
            return false;
        }
        ++str;
    }

    return true;
}

// Constant Folding Optimization
int constantFolding(TAC **head)
{
    printf(" 4 ");
    int changes = 0;
    TAC *current = *head;

    while (current != NULL)
    {
        if (current->op != NULL && (strcmp(current->op, "+") == 0 || strcmp(current->op, "-") == 0 ||
                                    strcmp(current->op, "*") == 0 || strcmp(current->op, "/") == 0))
        {
            // If both operands are constants
            if (isConstant(current->arg1) && isConstant(current->arg2))
            {
                int operand1 = atoi(current->arg1);
                int operand2 = atoi(current->arg2);
                int result = 0;

                if (strcmp(current->op, "+") == 0)
                {
                    result = operand1 + operand2;
                }
                else if (strcmp(current->op, "-") == 0)
                {
                    result = operand1 - operand2;
                }
                else if (strcmp(current->op, "*") == 0)
                {
                    result = operand1 * operand2;
                }
                else if (strcmp(current->op, "/") == 0)
                {
                    if (operand2 != 0)
                    {
                        result = operand1 / operand2;
                    }
                    else
                    {
                        fprintf(stderr, "Error: Division by zero\n");
                        current = current->next;
                        continue;
                    }
                }

                char resultStr[20];
                sprintf(resultStr, "%d", result);

                // Free old operands and operator
                free(current->arg1);
                free(current->arg2);
                free(current->op);

                // Update TAC node to assignment with the computed constant
                current->arg1 = strdup(resultStr);
                current->op = strdup("=");
                current->arg2 = NULL;

                changes++;
            }
        }

        current = current->next;
    }
    return changes;
}

// Constant Propagation Optimization
int constantPropagation(TAC **head)
{
    printf(" 5 ");
    int changes = 0;
    TAC *current = *head;
    while (current != NULL)
    {
        if (current->op != NULL && strcmp(current->op, "=") == 0)
        {
            // Check if the argument is a constant
            if (isConstant(current->arg1))
            {
                // Propagate the constant value to all uses of the variable until it's redefined
                char *constValue = current->arg1;
                char *varName = current->result;
                TAC *temp = current->next;
                while (temp != NULL)
                {
                    if (temp->result != NULL && strcmp(temp->result, varName) == 0)
                    {
                        // Variable is redefined
                        break;
                    }
                    if (temp->arg1 != NULL && strcmp(temp->arg1, varName) == 0)
                    {
                        free(temp->arg1);
                        temp->arg1 = strdup(constValue);
                        changes++;
                    }
                    if (temp->arg2 != NULL && strcmp(temp->arg2, varName) == 0)
                    {
                        free(temp->arg2);
                        temp->arg2 = strdup(constValue);
                        changes++;
                    }
                    temp = temp->next;
                }
            }
        }
        current = current->next;
    }
    return changes;
}

// Copy Propagation Optimization
int copyPropagation(TAC **head)
{
    printf(" 6 ");
    int changes = 0;
    TAC *current = *head;
    while (current != NULL)
    {
        if (current->op != NULL && strcmp(current->op, "=") == 0)
        {
            // Check if arg1 is a variable
            if (isVariable(current->arg1))
            {
                // Propagate the variable value to all uses of the variable until it's redefined
                char *sourceVar = current->arg1;
                char *destVar = current->result;
                TAC *temp = current->next;
                while (temp != NULL)
                {
                    if (temp->result != NULL && strcmp(temp->result, destVar) == 0)
                    {
                        // Variable is redefined
                        break;
                    }
                    if (temp->arg1 != NULL && strcmp(temp->arg1, destVar) == 0)
                    {
                        free(temp->arg1);
                        temp->arg1 = strdup(sourceVar);
                        changes++;
                    }
                    if (temp->arg2 != NULL && strcmp(temp->arg2, destVar) == 0)
                    {
                        free(temp->arg2);
                        temp->arg2 = strdup(sourceVar);
                        changes++;
                    }
                    temp = temp->next;
                }
            }
        }
        current = current->next;
    }
    return changes;
}

// Dead Code Elimination Optimization
int deadCodeElimination(TAC **head)
{
    printf(" 7 ");
    int changes = 0;
    TAC *current = *head;
    TAC *prev = NULL;

    while (current != NULL)
    {
        int isUsed = 0;
        if (current->result != NULL)
        {
            // Check if the instruction has side effects
            if (hasSideEffect(current))
            {
                // Do not remove instructions with side effects
                prev = current;
                current = current->next;
                continue;
            }

            // Check if the result is used in any of the following instructions before being redefined
            TAC *temp = current->next;
            while (temp != NULL)
            {
                // If the variable is used
                if ((temp->arg1 != NULL && strcmp(temp->arg1, current->result) == 0) ||
                    (temp->arg2 != NULL && strcmp(temp->arg2, current->result) == 0))
                {
                    isUsed = 1;
                    break;
                }
                // If the variable is redefined
                if (temp->result != NULL && strcmp(temp->result, current->result) == 0)
                {
                    break;
                }
                temp = temp->next;
            }
            if (!isUsed)
            {
                // Remove current instruction from the linked list
                TAC *toDelete = current;

                if (prev == NULL)
                {
                    // Removing the head of the list
                    *head = current->next;
                }
                else
                {
                    // Bypass the current node
                    prev->next = current->next;
                }

                // Move to the next instruction
                current = current->next;

                // Free the memory allocated for the instruction
                if (toDelete->op) free(toDelete->op);
                if (toDelete->arg1) free(toDelete->arg1);
                if (toDelete->arg2) free(toDelete->arg2);
                if (toDelete->result) free(toDelete->result);
                free(toDelete);

                changes++;
                continue; // Skip prev update
            }
        }
        prev = current;
        current = current->next;
    }
    return changes;
}


// Print the optimized TAC list to a file
void printOptimizedTAC(const char *filename, TAC *head)
{
    printf(" 8 ");
    FILE *outputFile = fopen(filename, "w");
    if (outputFile == NULL)
    {
        perror("Failed to open output file");
        exit(EXIT_FAILURE);
    }

    TAC *current = head;
    while (current != NULL)
    {
        if (current->result != NULL)
            fprintf(outputFile, "%s = ", current->result);
        if (current->arg1 != NULL)
            fprintf(outputFile, "%s ", current->arg1);
        if (current->op != NULL && (strcmp(current->op, "=") != 0))
            fprintf(outputFile, "%s ", current->op);
        if (current->arg2 != NULL)
            fprintf(outputFile, "%s ", current->arg2);
        fprintf(outputFile, "\n");
        current = current->next;
    }
    printf("Optimized TAC written to %s\n", filename);
    fclose(outputFile);
}

// Print current TAC instruction
void printCurrentOptimizedTAC(TAC *current)
{
    printf(" 9 ");
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

bool hasSideEffect(TAC *instr)
{
    printf(" 10 ");
    if (instr == NULL || instr->op == NULL)
        return false;

    // Instructions that modify memory or have side effects
    if (strcmp(instr->op, "[]=") == 0)    // Array assignment
        return true;
    if (strcmp(instr->op, "write") == 0)  // Write operation
        return true;
    // Add other side-effecting operations if needed

    return false;
}
