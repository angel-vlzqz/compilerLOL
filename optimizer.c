#include "optimizer.h"
#include "utils.h"
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

void optimizeTAC(TAC **head)
{
    printf("run optimizer\n");
    TAC *current = *head;

    while (current != NULL)
    {
        // Find the start of a function (prologue)
        if (current->op && strcmp(current->op, "prologue") == 0)
        {
            // This is the start of a function block
            TAC *funcStart = current;
            TAC *funcEnd = funcStart;

            // Traverse until we find the corresponding epilogue
            while (funcEnd != NULL && !(funcEnd->op && strcmp(funcEnd->op, "epilogue") == 0))
            {
                funcEnd = funcEnd->next;
            }

            // If no epilogue found, we can't properly isolate a function block.
            // Just break out, as we can't safely optimize.
            if (funcEnd == NULL)
            {
                break;
            }

            // Now [funcStart ... funcEnd] is the function block
            // We'll optimize only within these boundaries.

            int changes;
            int iterationCount = 0;
            const int MAX_ITERATIONS = 100;

            // Run optimization passes in a loop until no changes or max iterations
            do
            {
                changes = 0;
                // If you want to keep arithmetic steps visible, comment out the next line:
                changes += constantFolding(&funcStart); 
                changes += constantPropagation(&funcStart);
                changes += copyPropagation(&funcStart);
                changes += deadCodeElimination(&funcStart);
                iterationCount++;
                if (iterationCount >= MAX_ITERATIONS)
                {
                    fprintf(stderr, "Warning: Optimization reached %d iterations for a function and stopped.\n", MAX_ITERATIONS);
                    break;
                }
            } while (changes > 0 && iterationCount < MAX_ITERATIONS);

            // After optimization, funcStart might have changed
            // If we optimized the top instructions of the function, funcStart might now be different
            // We need to ensure the head is updated if needed
            if (*head == current)
                *head = funcStart;

            // Move current pointer just past the epilogue for the next iteration
            current = funcEnd->next;
        }
        else
        {
            // Not a prologue, just move to the next TAC instruction
            current = current->next;
        }
    }
}

// Constant Folding Optimization
int constantFolding(TAC **head)
{
    int changes = 0;
    TAC *current = *head;

    while (current != NULL)
    {
        if (current->op && (strcmp(current->op, "prologue") == 0 || strcmp(current->op, "epilogue") == 0 ||
                            strcmp(current->op, "call") == 0 || strcmp(current->op, "return") == 0 ||
                            strcmp(current->op, "ifFalse") == 0 || strcmp(current->op, "goto") == 0 ||
                            strcmp(current->op, "label") == 0 || strcmp(current->op, "param") == 0 ||
                            strcmp(current->op, "write") == 0 || strcmp(current->op, "write_float") == 0 ||
                            strcmp(current->op, "=[]") == 0 || strcmp(current->op, "[]=") == 0))
        {
            // Skip these operations for constant folding
            current = current->next;
            continue;
        }

        // Fold simple arithmetic operations where both args are constants
        if (current->op && (strcmp(current->op, "+") == 0 || strcmp(current->op, "-") == 0 ||
                            strcmp(current->op, "*") == 0 || strcmp(current->op, "/") == 0))
        {
            if (isConstant(current->arg1) && isConstant(current->arg2))
            {
                int operand1 = atoi(current->arg1);
                int operand2 = atoi(current->arg2);
                int result = 0;
                bool divisionByZero = false;

                if (strcmp(current->op, "+") == 0)
                    result = operand1 + operand2;
                else if (strcmp(current->op, "-") == 0)
                    result = operand1 - operand2;
                else if (strcmp(current->op, "*") == 0)
                    result = operand1 * operand2;
                else if (strcmp(current->op, "/") == 0)
                {
                    if (operand2 == 0)
                        divisionByZero = true;
                    else
                        result = operand1 / operand2;
                }

                if (!divisionByZero)
                {
                    char resultStr[20];
                    sprintf(resultStr, "%d", result);

                    free(current->arg1);
                    free(current->arg2);
                    free(current->op);

                    current->arg1 = strdup(resultStr);
                    current->op = strdup("=");
                    current->arg2 = NULL;

                    changes++;
                }
                else
                {
                    fprintf(stderr, "Error: Division by zero in constant folding.\n");
                }
            }
        }
        current = current->next;
    }

    return changes;
}

// Constant Propagation
int constantPropagation(TAC **head)
{
    int changes = 0;
    TAC *current = *head;
    while (current != NULL)
    {
        if (current->op && strcmp(current->op, "=") == 0 && isConstant(current->arg1))
        {
            char *constValue = current->arg1;
            char *varName = current->result;
            TAC *temp = current->next;

            while (temp != NULL)
            {
                if (temp->op && (strcmp(temp->op, "prologue") == 0 || strcmp(temp->op, "epilogue") == 0 || strcmp(temp->op, "call") == 0))
                    break;

                if (temp->arg1 && strcmp(temp->arg1, varName) == 0)
                {
                    free(temp->arg1);
                    temp->arg1 = strdup(constValue);
                    changes++;
                }
                if (temp->arg2 && strcmp(temp->arg2, varName) == 0)
                {
                    free(temp->arg2);
                    temp->arg2 = strdup(constValue);
                    changes++;
                }

                if (temp->result && strcmp(temp->result, varName) == 0)
                    break;

                temp = temp->next;
            }
        }
        current = current->next;
    }
    return changes;
}

// Copy Propagation
int copyPropagation(TAC **head)
{
    int changes = 0;
    TAC *current = *head;
    while (current != NULL)
    {
        if (current->op && strcmp(current->op, "=") == 0 && isVariable(current->arg1))
        {
            char *sourceVar = current->arg1;
            char *destVar = current->result;
            TAC *temp = current->next;

            while (temp != NULL)
            {
                if (temp->op && (strcmp(temp->op, "prologue") == 0 || strcmp(temp->op, "epilogue") == 0 || strcmp(temp->op, "call") == 0))
                    break;

                if (temp->arg1 && strcmp(temp->arg1, destVar) == 0)
                {
                    free(temp->arg1);
                    temp->arg1 = strdup(sourceVar);
                    changes++;
                }
                if (temp->arg2 && strcmp(temp->arg2, destVar) == 0)
                {
                    free(temp->arg2);
                    temp->arg2 = strdup(sourceVar);
                    changes++;
                }

                if (temp->result && strcmp(temp->result, destVar) == 0)
                    break;

                temp = temp->next;
            }
        }
        current = current->next;
    }
    return changes;
}

// Dead Code Elimination
int deadCodeElimination(TAC **head)
{
    int changes = 0;
    TAC *current = *head;
    TAC *prev = NULL;

    while (current != NULL)
    {
        if (current->op && (strcmp(current->op, "prologue") == 0 || strcmp(current->op, "epilogue") == 0))
        {
            prev = current;
            current = current->next;
            continue;
        }

        if (current->result && !hasSideEffect(current))
        {
            int isUsed = 0;
            TAC *temp = current->next;
            while (temp != NULL && !(temp->op && strcmp(temp->op, "epilogue") == 0))
            {
                if ((temp->arg1 && strcmp(temp->arg1, current->result) == 0) ||
                    (temp->arg2 && strcmp(temp->arg2, current->result) == 0))
                {
                    isUsed = 1;
                    break;
                }

                if (temp->result && strcmp(temp->result, current->result) == 0)
                    break;

                temp = temp->next;
            }

            if (!isUsed)
            {
                TAC *toDelete = current;

                if (prev == NULL)
                    *head = current->next;
                else
                    prev->next = current->next;

                current = current->next;

                if (toDelete->op) free(toDelete->op);
                if (toDelete->arg1) free(toDelete->arg1);
                if (toDelete->arg2) free(toDelete->arg2);
                if (toDelete->result) free(toDelete->result);
                free(toDelete);

                changes++;
                continue;
            }
        }
        prev = current;
        current = current->next;
    }
    return changes;
}

bool hasSideEffect(TAC *instr)
{
    if (instr == NULL || instr->op == NULL)
        return false;

    // Instructions with side effects or special operations
    if (strcmp(instr->op, "[]=") == 0)
        return true; // Array assignment modifies memory
    if (strcmp(instr->op, "write") == 0 || strcmp(instr->op, "write_float") == 0)
        return true; // Output side effect
    if (strcmp(instr->op, "call") == 0)
        return true; // Function call may have side effects
    if (strcmp(instr->op, "prologue") == 0)
        return true;
    if (strcmp(instr->op, "epilogue") == 0)
        return true;
    if (strcmp(instr->op, "return") == 0)
        return true; // Return from function changes control flow

    return false;
}
