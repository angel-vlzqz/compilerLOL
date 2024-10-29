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
        // Find start of function (prologue)
        if (current->op != NULL && strcmp(current->op, "prologue") == 0)
        {
            // Isolate function-specific TAC from prologue to epilogue
            TAC *funcStart = current;
            TAC *funcEnd = funcStart;

            // Traverse until end of function (epilogue)
            while (funcEnd != NULL && !(funcEnd->op != NULL && strcmp(funcEnd->op, "epilogue") == 0))
            {
                funcEnd = funcEnd->next;
            }

            // Apply optimizations within this function block
            int changes;
            do
            {
                changes = 0;
                changes += constantFolding(&funcStart);
                changes += constantPropagation(&funcStart);
                changes += copyPropagation(&funcStart);
                changes += deadCodeElimination(&funcStart);
            } while (changes > 0);

            // Continue optimization with the next function
            current = funcEnd;
        }
        current = current->next;
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
                            strcmp(current->op, "call") == 0 || strcmp(current->op, "return") == 0))
        {
            current = current->next;
            continue;
        }

        if (current->op && (strcmp(current->op, "+") == 0 || strcmp(current->op, "-") == 0 ||
                            strcmp(current->op, "*") == 0 || strcmp(current->op, "/") == 0))
        {
            if (isConstant(current->arg1) && isConstant(current->arg2))
            {
                int operand1 = atoi(current->arg1);
                int operand2 = atoi(current->arg2);
                int result = 0;

                if (strcmp(current->op, "+") == 0) result = operand1 + operand2;
                else if (strcmp(current->op, "-") == 0) result = operand1 - operand2;
                else if (strcmp(current->op, "*") == 0) result = operand1 * operand2;
                else if (strcmp(current->op, "/") == 0 && operand2 != 0) result = operand1 / operand2;

                if (strcmp(current->op, "/") == 0 && operand2 == 0) {
                    fprintf(stderr, "Error: Division by zero\n");
                    current = current->next;
                    continue;
                }

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
        }
        current = current->next;
    }
    return changes;
}

// Constant Propagation Optimization
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

                if (temp->result && strcmp(temp->result, varName) == 0)
                    break;

                if (temp->arg1 && strcmp(temp->arg1, varName) == 0) {
                    free(temp->arg1);
                    temp->arg1 = strdup(constValue);
                    changes++;
                }
                if (temp->arg2 && strcmp(temp->arg2, varName) == 0) {
                    free(temp->arg2);
                    temp->arg2 = strdup(constValue);
                    changes++;
                }
                temp = temp->next;
            }
        }
        current = current->next;
    }
    return changes;
}

// Copy Propagation Optimization
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

                if (temp->result && strcmp(temp->result, destVar) == 0)
                    break;

                if (temp->arg1 && strcmp(temp->arg1, destVar) == 0) {
                    free(temp->arg1);
                    temp->arg1 = strdup(sourceVar);
                    changes++;
                }
                if (temp->arg2 && strcmp(temp->arg2, destVar) == 0) {
                    free(temp->arg2);
                    temp->arg2 = strdup(sourceVar);
                    changes++;
                }
                temp = temp->next;
            }
        }
        current = current->next;
    }
    return changes;
}


// Dead Code Elimination Optimization
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
                free(toDelete->op);
                free(toDelete->arg1);
                free(toDelete->arg2);
                free(toDelete->result);
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

    // Instructions with side effects or function-related operations
    if (strcmp(instr->op, "[]=") == 0)
        return true; // Array assignment
    if (strcmp(instr->op, "write") == 0)
        return true; // Write operation
    if (strcmp(instr->op, "call") == 0)
        return true; // Function call
    if (strcmp(instr->op, "  prologue") == 0)
        return true; // Function prologue
    if (strcmp(instr->op, "  epilogue") == 0)
        return true; // Function epilogue
    if (strcmp(instr->op, "  return") == 0)
        return true; // Function return

    return false;
}