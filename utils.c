// utils.c
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// ---- Error Helper ----

void fatal(const char *s) 
{
    fprintf(stderr, "Fatal Error: %s\n", s);
    exit(1);  // Exit the program with a non-zero status
}

// ---- optimizer.c Helpers ----

bool isConstant(const char* str) {
    if (str == NULL || *str == '\0') {
        return false;
    }
    if (*str == '-') ++str;
    while (*str) {
        if (!isdigit((unsigned char)*str)) return false;
        ++str;
    }
    return true;
}

bool isVariable(const char* str) {
    if (str == NULL || *str == '\0') return false;
    if (!isalpha((unsigned char)*str) && *str != '_') return false;
    ++str;
    while (*str) {
        if (!isalnum((unsigned char)*str) && *str != '_') return false;
        ++str;
    }
    return true;
}

// ---- semantic.c Helpers ----

void initializeTempVars()
{
    for (int i = 0; i < 50; i++)
    {
        tempVars[i] = 0;
    }
}

int allocateNextAvailableTempVar(int tempVars[], int size)
{
    for (int i = 0; i < size; i++)
    {
        if (tempVars[i] == 0)
        {
            tempVars[i] = 1;
            return i;
        }
    }
    return -1; // No available temp var
}

void deallocateTempVar(int tempVars[], int index)
{
    if (index >= 0 && index < 50)
    {
        tempVars[index] = 0;
    }
}

void printTACToFile(const char *filename, TAC *tac)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        perror("Failed to open file");
        return;
    }

    TAC *current = tac;

    while (current != NULL)
    {
        if (current->op != NULL)
        {
            if (strcmp(current->op, "=") == 0)
            {
                fprintf(file, "%s = %s\n", current->result ? current->result : "", current->arg1 ? current->arg1 : "");
            }
            else if (strcmp(current->op, "write") == 0)
            {
                fprintf(file, "write %s\n", current->arg1 ? current->arg1 : "");
            }
            else if (strcmp(current->op, "write_float") == 0)
            {
                fprintf(file, "write %s\n", current->arg1 ? current->arg1 : "");
            }
            else if (strcmp(current->op, "[]=") == 0)
            {
                fprintf(file, "%s [ %s ] = %s\n", current->result ? current->result : "", current->arg1 ? current->arg1 : "", current->arg2 ? current->arg2 : "");
            }
            else if (strcmp(current->op, "=[]") == 0)
            {
                fprintf(file, "%s = %s [ %s ]\n", current->result ? current->result : "", current->arg1 ? current->arg1 : "", current->arg2 ? current->arg2 : "");
            }
            else if (strcmp(current->op, "label") == 0)
            {
                fprintf(file, "\n%s:\n", current->arg1 ? current->arg1 : ""); // Label format
            }
            else if (strcmp(current->op, "prologue") == 0)
            {
                fprintf(file, "  prologue for %s\n", current->arg1 ? current->arg1 : ""); // Prologue format
            }
            else if (strcmp(current->op, "epilogue") == 0)
            {
                fprintf(file, "  epilogue for %s\n", current->arg1 ? current->arg1 : ""); // Epilogue format
            }
            else if (strcmp(current->op, "return") == 0)
            {
                fprintf(file, "  return %s\n", current->result ? current->result : "void"); // Return format
            }
            else if (strcmp(current->op, "call") == 0)
            {
                // TAC for a function call with or without result assignment
                if (current->result)
                    fprintf(file, "%s = call %s\n", current->result, current->arg1 ? current->arg1 : "");
                else
                    fprintf(file, "call %s\n", current->arg1 ? current->arg1 : "");
            }
            else if (strcmp(current->op, "param") == 0)
            {
                // TAC for passing parameters to a function call
                fprintf(file, "param %s\n", current->arg1 ? current->arg1 : "");
            }
            else
            {
                fprintf(file, "%s = %s %s %s\n",
                        current->result ? current->result : "",
                        current->arg1 ? current->arg1 : "",
                        current->op,
                        current->arg2 ? current->arg2 : "");
            }
        }
        current = current->next;
    }

    fclose(file);
}