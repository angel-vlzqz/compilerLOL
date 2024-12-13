#include "optimizer.h"
#include "utils.h"
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

// Helper: Check if a string is a numeric constant (integer or float)
static bool isNumericConstant(const char *str)
{
    if (!str || *str == '\0')
        return false;
    const char *p = str;
    if (*p == '+' || *p == '-')
        p++;

    bool hasDigit = false;
    bool hasDot = false;
    while (*p)
    {
        if (*p == '.')
        {
            if (hasDot)
                return false;
            hasDot = true;
        }
        else if (!isdigit((unsigned char)*p))
        {
            return false;
        }
        else
        {
            hasDigit = true;
        }
        p++;
    }

    return hasDigit;
}

// Parse a numeric constant as double
static double parseDouble(const char *str)
{
    errno = 0;
    double val = strtod(str, NULL);
    return val;
}

void optimizeTAC(TAC **head)
{
    printf("run optimizer\n");
    TAC *current = *head;

    while (current != NULL)
    {
        // Find start of function block
        if (current->op && strcmp(current->op, "prologue") == 0)
        {
            TAC *funcStart = current;
            TAC *funcEnd = funcStart;

            // Find corresponding epilogue
            while (funcEnd != NULL && !(funcEnd->op && strcmp(funcEnd->op, "epilogue") == 0))
            {
                funcEnd = funcEnd->next;
            }

            if (funcEnd == NULL)
                break;

            int changes;
            int iterationCount = 0;
            const int MAX_ITERATIONS = 100;

            do
            {
                changes = 0;
                changes += constantFolding(&funcStart);
                changes += constantPropagation(&funcStart);
                changes += copyPropagation(&funcStart);
                changes += deadCodeElimination(&funcStart);
                iterationCount++;
                if (iterationCount >= MAX_ITERATIONS)
                {
                    fprintf(stderr, "Warning: Optimization reached %d iterations and stopped.\n", MAX_ITERATIONS);
                    break;
                }
            } while (changes > 0 /*&& iterationCount < MAX_ITERATIONS*/);

            if (*head == current)
                *head = funcStart;

            current = funcEnd->next;
        }
        else
        {
            current = current->next;
        }
    }
}

int constantFolding(TAC **head)
{
    int changes = 0;
    TAC *current = *head;

    while (current != NULL)
    {
        // Skip complex ops or those that must not be folded
        if (current->op && (
            strcmp(current->op, "prologue") == 0 ||
            strcmp(current->op, "epilogue") == 0 ||
            strcmp(current->op, "call") == 0 ||
            strcmp(current->op, "return") == 0 ||
            strcmp(current->op, "ifFalse") == 0 ||
            strcmp(current->op, "goto") == 0 ||
            strcmp(current->op, "label") == 0 ||
            strcmp(current->op, "param") == 0 ||
            strcmp(current->op, "write") == 0 ||
            strcmp(current->op, "write_float") == 0 ||
            strcmp(current->op, "=[]") == 0 ||
            strcmp(current->op, "[]=") == 0 ||
            strcmp(current->op, "!") == 0 ||
            strcmp(current->op, "&&") == 0 ||
            strcmp(current->op, "||") == 0 ||
            strcmp(current->op, "==") == 0 ||
            strcmp(current->op, "!=") == 0 ||
            strcmp(current->op, "<") == 0 ||
            strcmp(current->op, "<=") == 0 ||
            strcmp(current->op, ">") == 0 ||
            strcmp(current->op, ">=") == 0))
        {
            current = current->next;
            continue;
        }

        // Integer and float operations: +, -, *, /
        if (current->op && (strcmp(current->op, "+") == 0 ||
                            strcmp(current->op, "-") == 0 ||
                            strcmp(current->op, "*") == 0 ||
                            strcmp(current->op, "/") == 0))
        {
            if (isNumericConstant(current->arg1) && isNumericConstant(current->arg2))
            {
                bool isFloatArg = (strchr(current->arg1, '.') != NULL) || (strchr(current->arg2, '.') != NULL);
                if (!isFloatArg)
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
                else
                {
                    // float folding
                    double operand1 = parseDouble(current->arg1);
                    double operand2 = parseDouble(current->arg2);
                    double result = 0.0;
                    bool divisionByZero = false;

                    if (strcmp(current->op, "+") == 0)
                        result = operand1 + operand2;
                    else if (strcmp(current->op, "-") == 0)
                        result = operand1 - operand2;
                    else if (strcmp(current->op, "*") == 0)
                        result = operand1 * operand2;
                    else if (strcmp(current->op, "/") == 0)
                    {
                        if (operand2 == 0.0)
                            divisionByZero = true;
                        else
                            result = operand1 / operand2;
                    }

                    if (!divisionByZero)
                    {
                        char resultStr[64];
                        sprintf(resultStr, "%f", result);

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
                        fprintf(stderr, "Error: Division by zero in float constant folding.\n");
                    }
                }
            }
        }
        else if (current->op && (strcmp(current->op, "fadd") == 0 ||
                                 strcmp(current->op, "fsub") == 0 ||
                                 strcmp(current->op, "fmul") == 0 ||
                                 strcmp(current->op, "fdiv") == 0))
        {
            if (isNumericConstant(current->arg1) && isNumericConstant(current->arg2))
            {
                double operand1 = parseDouble(current->arg1);
                double operand2 = parseDouble(current->arg2);
                double result = 0.0;
                bool divisionByZero = false;

                if (strcmp(current->op, "fadd") == 0)
                    result = operand1 + operand2;
                else if (strcmp(current->op, "fsub") == 0)
                    result = operand1 - operand2;
                else if (strcmp(current->op, "fmul") == 0)
                    result = operand1 * operand2;
                else if (strcmp(current->op, "fdiv") == 0)
                {
                    if (operand2 == 0.0)
                        divisionByZero = true;
                    else
                        result = operand1 / operand2;
                }

                if (!divisionByZero)
                {
                    char resultStr[64];
                    sprintf(resultStr, "%f", result);

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
                    fprintf(stderr, "Error: Division by zero in float constant folding.\n");
                }
            }
        }

        current = current->next;
    }

    return changes;
}

int constantPropagation(TAC **head)
{
    int changes = 0;
    TAC *current = *head;
    while (current != NULL)
    {
        if (current->op && strcmp(current->op, "=") == 0 && isNumericConstant(current->arg1))
        {
            char *constValue = current->arg1;
            char *varName = current->result;
            TAC *temp = current->next;

            while (temp != NULL)
            {
                if (temp->op && (strcmp(temp->op, "prologue") == 0 ||
                                 strcmp(temp->op, "epilogue") == 0 ||
                                 strcmp(temp->op, "call") == 0))
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
                if (temp->op && (strcmp(temp->op, "prologue") == 0 ||
                                 strcmp(temp->op, "epilogue") == 0 ||
                                 strcmp(temp->op, "call") == 0))
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

int deadCodeElimination(TAC **head)
{
    int changes = 0;
    TAC *current = *head;
    TAC *prev = NULL;

    while (current != NULL)
    {
        if (current->op && (strcmp(current->op, "prologue") == 0 ||
                            strcmp(current->op, "epilogue") == 0))
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

    // If there is a return TAC, it always has a side effect
    if (strcmp(instr->op, "return") == 0) return true;

    // If your function uses 'result' as the final return variable
    // and does not generate a 'return' TAC, treat assignments to 'result' as side effects:
    if (instr->result && strcmp(instr->result, "result") == 0)
        return true;

    if (strcmp(instr->op, "[]=") == 0) return true;
    if (strcmp(instr->op, "write") == 0 || strcmp(instr->op, "write_float") == 0) return true;
    if (strcmp(instr->op, "call") == 0) return true;
    if (strcmp(instr->op, "prologue") == 0) return true;
    if (strcmp(instr->op, "epilogue") == 0) return true;
    if (strcmp(instr->op, "param") == 0) return true;
    if (strcmp(instr->op, "label") == 0) return true;
    if (strcmp(instr->op, "ifFalse") == 0) return true;
    if (strcmp(instr->op, "goto") == 0) return true;

    // Arithmetic and float arithmetic, relational, logical ops, and '=' are not side effects if unused.
    return false;
}
