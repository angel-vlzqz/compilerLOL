#include "optimizer.h"
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

void optimizeTAC(TAC **head)
{
    constantFolding(head);
    constantPropagation(head);
    copyPropagation(head);
    deadCodeElimination(head);
}

bool isConstant(const char *str)
{
    if (str == NULL || *str == '\0')
    {
        return false; // Empty string is not a constant
    }

    // Optional: Handle negative numbers
    if (*str == '-')
    {
        ++str;
    }

    // Check if string is numeric
    while (*str)
    {
        if (!isdigit((unsigned char)*str))
        {
            return false; // Found a non-digit character
        }
        ++str;
    }

    return true; // All characters were digits
}

bool isVariable(const char *str)
{
    if (str == NULL || *str == '\0')
    {
        return false; // Null or empty string is not a variable
    }

    // Check if the first character is a letter or underscore
    if (!isalpha((unsigned char)*str) && *str != '_')
    {
        return false;
    }

    // Check remaining characters for letters, digits, or underscores
    ++str; // Move past the first character
    while (*str)
    {
        if (!isalnum((unsigned char)*str) && *str != '_')
        {
            return false; // Invalid character found
        }
        ++str;
    }

    return true; // String meets the criteria for a variable name
}

// Constant Folding Optimization
void constantFolding(TAC **head)
{
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
                        return;
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
            }
        }

        current = current->next;
    }
}

// Constant Propagation Optimization
void constantPropagation(TAC **head)
{
    TAC *current = *head;
    while (current != NULL)
    {
        if (current->op != NULL && strcmp(current->op, "=") == 0)
        {
            // Check if the argument is a constant
            if (isConstant(current->arg1))
            {
                // Propagate the constant value to all uses of the variable
                char *constValue = current->arg1;
                char *varName = current->result;
                TAC *temp = current->next;
                while (temp != NULL)
                {
                    if (temp->arg1 != NULL && strcmp(temp->arg1, varName) == 0)
                    {
                        free(temp->arg1);
                        temp->arg1 = strdup(constValue);
                    }
                    if (temp->arg2 != NULL && strcmp(temp->arg2, varName) == 0)
                    {
                        free(temp->arg2);
                        temp->arg2 = strdup(constValue);
                    }
                    temp = temp->next;
                }
            }
        }
        current = current->next;
    }
}

// Copy Propagation Optimization
void copyPropagation(TAC **head)
{
    TAC *current = *head;
    while (current != NULL)
    {
        if (current->op != NULL && strcmp(current->op, "=") == 0)
        {
            // Check if arg1 is a variable
            if (isVariable(current->arg1))
            {
                // Propagate the variable value to all uses of the variable
                char *sourceVar = current->arg1;
                char *destVar = current->result;
                TAC *temp = current->next;
                while (temp != NULL)
                {
                    if (temp->arg1 != NULL && strcmp(temp->arg1, destVar) == 0)
                    {
                        free(temp->arg1);
                        temp->arg1 = strdup(sourceVar);
                    }
                    if (temp->arg2 != NULL && strcmp(temp->arg2, destVar) == 0)
                    {
                        free(temp->arg2);
                        temp->arg2 = strdup(sourceVar);
                    }
                    temp = temp->next;
                }
            }
        }
        current = current->next;
    }
}

// Dead Code Elimination Optimization
void deadCodeElimination(TAC **head)
{
    TAC *current = *head;
    TAC *prev = NULL;

    while (current != NULL)
    {
        int isUsed = 0;
        if (current->result != NULL)
        {
            // Check if the result is used in any of the following instructions
            TAC *temp = current->next;
            while (temp != NULL)
            {
                if ((temp->arg1 != NULL && strcmp(temp->arg1, current->result) == 0) ||
                    (temp->arg2 != NULL && strcmp(temp->arg2, current->result) == 0))
                {
                    isUsed = 1;
                    break;
                }
                temp = temp->next;
            }
            if (!isUsed)
            {
                // Remove current instruction
                if (prev == NULL)
                {
                    // Removing the head
                    *head = current->next;
                    // Free current node
                    free(current->op);
                    free(current->arg1);
                    free(current->arg2);
                    free(current->result);
                    TAC *toDelete = current;
                    current = current->next;
                    free(toDelete);
                    continue;
                }
                else
                {
                    prev->next = current->next;
                    // Free current node
                    free(current->op);
                    free(current->arg1);
                    free(current->arg2);
                    free(current->result);
                    TAC *toDelete = current;
                    current = current->next;
                    free(toDelete);
                    continue;
                }
            }
        }
        prev = current;
        current = current->next;
    }
}

// Print the optimized TAC list to a file
void printOptimizedTAC(const char *filename, TAC *head)
{
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
        if (current->op != NULL && (strcmp(current->op, "=") != 0)) // Skip printing the "=" operator
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