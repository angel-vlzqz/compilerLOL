#include "optimizer.h"
#include "utils.h"
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

// Structures to hold known constants for variables and array elements
typedef struct VarConst {
    char *var;
    char *constant;
    struct VarConst *next;
} VarConst;

typedef struct ArrConst {
    char *arrElement;   // "arr[3]"
    char *constant;
    struct ArrConst *next;
} ArrConst;

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

static double parseDouble(const char *str)
{
    errno = 0;
    double val = strtod(str, NULL);
    return val;
}

static const char* findVarConstant(VarConst *list, const char *var) {
    for (VarConst *v = list; v; v = v->next) {
        if (strcmp(v->var, var) == 0) return v->constant;
    }
    return NULL;
}

static void setVarConstant(VarConst **list, const char *var, const char *constant) {
    VarConst *prev = NULL, *cur = *list;
    while (cur) {
        if (strcmp(cur->var, var) == 0) {
            free(cur->constant);
            cur->constant = strdup(constant);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
    VarConst *newNode = (VarConst*)malloc(sizeof(VarConst));
    newNode->var = strdup(var);
    newNode->constant = strdup(constant);
    newNode->next = *list;
    *list = newNode;
}

static void invalidateVarConstant(VarConst **list, const char *var) {
    VarConst *prev = NULL, *cur = *list;
    while (cur) {
        if (strcmp(cur->var, var) == 0) {
            if (prev) prev->next = cur->next; else *list = cur->next;
            free(cur->var);
            free(cur->constant);
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

static const char* findArrConstant(ArrConst *list, const char *arrElem) {
    for (ArrConst *a = list; a; a = a->next) {
        if (strcmp(a->arrElement, arrElem) == 0) return a->constant;
    }
    return NULL;
}

static void setArrConstant(ArrConst **list, const char *arrElem, const char *constant) {
    ArrConst *prev = NULL, *cur = *list;
    while (cur) {
        if (strcmp(cur->arrElement, arrElem) == 0) {
            free(cur->constant);
            cur->constant = strdup(constant);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
    ArrConst *newNode = (ArrConst*)malloc(sizeof(ArrConst));
    newNode->arrElement = strdup(arrElem);
    newNode->constant = strdup(constant);
    newNode->next = *list;
    *list = newNode;
}

static void invalidateArrConstant(ArrConst **list, const char *arrElem) {
    ArrConst *prev = NULL, *cur = *list;
    while (cur) {
        if (strcmp(cur->arrElement, arrElem) == 0) {
            if (prev) prev->next = cur->next; else *list = cur->next;
            free(cur->arrElement);
            free(cur->constant);
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
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
                do
                {
                    changes = 0;
                    changes += constantFolding(&funcStart);
                    changes += constantPropagation(&funcStart);
                    changes += copyPropagation(&funcStart);
                    iterationCount++;
                    if (iterationCount >= MAX_ITERATIONS)
                    {
                        fprintf(stderr, "Warning: Optimization reached %d iterations and stopped.\n", MAX_ITERATIONS);
                        break;
                    }
                } while (changes > 0);

                changes += deadCodeElimination(&funcStart);
                iterationCount++;
                if (iterationCount >= MAX_ITERATIONS)
                {
                    fprintf(stderr, "Warning: Optimization reached %d iterations and stopped.\n", MAX_ITERATIONS);
                    break;
                }
            } while (changes > 0);

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
                }
                else
                {
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
            }
        }

        current = current->next;
    }

    return changes;
}

int constantPropagation(TAC **head)
{
    int changes = 0;
    VarConst *varConstants = NULL;
    ArrConst *arrConstants = NULL;

    for (TAC *current = *head; current; current = current->next) {
        // Stop at function boundaries or calls
        if (current->op && (strcmp(current->op, "prologue") == 0 ||
                            strcmp(current->op, "epilogue") == 0 ||
                            strcmp(current->op, "call") == 0))
        {
            // Clear known constants at boundaries
            while (varConstants) {
                VarConst *vn = varConstants->next;
                free(varConstants->var);
                free(varConstants->constant);
                free(varConstants);
                varConstants = vn;
            }
            while (arrConstants) {
                ArrConst *an = arrConstants->next;
                free(arrConstants->arrElement);
                free(arrConstants->constant);
                free(arrConstants);
                arrConstants = an;
            }
            continue;
        }

        // First, propagate constants into arg1/arg2 if they are variables
        if (current->arg1) {
            const char *vc = findVarConstant(varConstants, current->arg1);
            if (vc && isNumericConstant(vc) && strcmp(vc, current->arg1) != 0) {
                free(current->arg1);
                current->arg1 = strdup(vc);
                changes++;
            }
        }

        if (current->arg2) {
            const char *vc = findVarConstant(varConstants, current->arg2);
            if (vc && isNumericConstant(vc) && strcmp(vc, current->arg2) != 0) {
                free(current->arg2);
                current->arg2 = strdup(vc);
                changes++;
            }
        }

        // If we have array instructions, try to propagate constants into the index
        if (current->op && strcmp(current->op, "=[]") == 0) {
            // var = arr[index]
            // Try to make index a constant if possible
            if (current->arg2) {
                const char *idxConst = findVarConstant(varConstants, current->arg2);
                if (idxConst && isNumericConstant(idxConst) && strcmp(idxConst, current->arg2) != 0) {
                    free(current->arg2);
                    current->arg2 = strdup(idxConst);
                    changes++;
                }
            }

            if (isNumericConstant(current->arg2)) {
                // Now try arr[index]
                char arrKey[256];
                snprintf(arrKey, sizeof(arrKey), "%s[%s]", current->arg1, current->arg2);
                const char *ac = findArrConstant(arrConstants, arrKey);
                if (ac && isNumericConstant(ac)) {
                    free(current->op);
                    current->op = strdup("=");
                    free(current->arg1);
                    current->arg1 = strdup(ac);
                    free(current->arg2);
                    current->arg2 = NULL;
                    changes++;

                    if (current->result) {
                        setVarConstant(&varConstants, current->result, ac);
                    }
                } else {
                    // Not known, invalidate var
                    if (current->result)
                        invalidateVarConstant(&varConstants, current->result);
                }
            } else {
                if (current->result)
                    invalidateVarConstant(&varConstants, current->result);
            }
        } else if (current->op && strcmp(current->op, "[]=") == 0) {
            // arr[index] = value
            // Try to make index a constant if possible
            if (current->arg2) {
                const char *idxConst = findVarConstant(varConstants, current->arg2);
                if (idxConst && isNumericConstant(idxConst) && strcmp(idxConst, current->arg2) != 0) {
                    free(current->arg2);
                    current->arg2 = strdup(idxConst);
                    changes++;
                }
            }

            if (isNumericConstant(current->arg2)) {
                char arrKey[256];
                snprintf(arrKey, sizeof(arrKey), "%s[%s]", current->arg1, current->arg2);
                if (current->result && isNumericConstant(current->result)) {
                    setArrConstant(&arrConstants, arrKey, current->result);
                } else {
                    // If result is a var, check if var is known constant
                    if (current->result) {
                        const char *vc = findVarConstant(varConstants, current->result);
                        if (vc && isNumericConstant(vc)) {
                            free(current->result);
                            current->result = strdup(vc);
                            changes++;
                            setArrConstant(&arrConstants, arrKey, vc);
                        } else {
                            invalidateArrConstant(&arrConstants, arrKey);
                        }
                    }
                }
            } else {
                // Unknown index
                // Can't set arr constant
                // If we had a known constant for arr[index], now it should be invalidated
                // But we don't know which index, so do nothing special here
            }
        } else if (current->op && strcmp(current->op, "=") == 0) {
            // var = something
            if (current->arg1 && isNumericConstant(current->arg1)) {
                if (current->result)
                    setVarConstant(&varConstants, current->result, current->arg1);
            } else {
                if (current->arg1) {
                    const char *vc = findVarConstant(varConstants, current->arg1);
                    if (vc && isNumericConstant(vc)) {
                        free(current->arg1);
                        current->arg1 = strdup(vc);
                        changes++;
                        setVarConstant(&varConstants, current->result, vc);
                    } else {
                        if (current->result)
                            invalidateVarConstant(&varConstants, current->result);
                    }
                } else {
                    if (current->result)
                        invalidateVarConstant(&varConstants, current->result);
                }
            }
        } else {
            // For other ops that define a result, it's likely not constant
            if (current->result && strcmp(current->op, "[]=") != 0 && strcmp(current->op, "=[]") != 0 && strcmp(current->op, "=") != 0) {
                invalidateVarConstant(&varConstants, current->result);
            }
        }

        // Final propagation step for arguments after possible updates
        if (current->arg1) {
            const char *vc = findVarConstant(varConstants, current->arg1);
            if (vc && isNumericConstant(vc) && strcmp(vc, current->arg1) != 0) {
                free(current->arg1);
                current->arg1 = strdup(vc);
                changes++;
            }
        }
        if (current->arg2) {
            const char *vc = findVarConstant(varConstants, current->arg2);
            if (vc && isNumericConstant(vc) && strcmp(vc, current->arg2) != 0) {
                free(current->arg2);
                current->arg2 = strdup(vc);
                changes++;
            }
        }

        // Check for []= result propagation
        if (current->op && strcmp(current->op, "[]=") == 0 && current->result) {
            const char *vc = findVarConstant(varConstants, current->result);
            if (vc && isNumericConstant(vc) && strcmp(vc, current->result) != 0) {
                free(current->result);
                current->result = strdup(vc);
                changes++;
            }
        }
    }

    // Free varConstants and arrConstants lists
    while (varConstants) {
        VarConst *vn = varConstants->next;
        free(varConstants->var);
        free(varConstants->constant);
        free(varConstants);
        varConstants = vn;
    }
    while (arrConstants) {
        ArrConst *an = arrConstants->next;
        free(arrConstants->arrElement);
        free(arrConstants->constant);
        free(arrConstants);
        arrConstants = an;
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

                // If the destination is redefined, stop propagation
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

    if (strcmp(instr->op, "return") == 0) return true;
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

    return false;
}
