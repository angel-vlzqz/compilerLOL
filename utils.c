// utils.c
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

const char *nodeTypeNames[] = {
    "Program",
    "DeclList",
    "FuncDeclList",
    "FuncDecl",
    "ParamList",
    "Param",
    "FuncCall",
    "ArgList",
    "Arg",
    "VarDeclList",
    "VarDecl",
    "ArrayDecl",
    "StmtList",
    "AssignStmt",
    "ArrayAssign",
    "BinOp",
    "SimpleID",
    "SimpleExpr",
    "ArrayAccess",
    "Block",
    "ReturnStmt",
    "WriteStmt",
    "IfStmt",
    "WhileStmt",
    "LogicalOp",
    "CastExpr",
    "RelOp",
    "NotOp"};

// ---- Error Helper ----

void fatal(const char *s)
{
    fprintf(stderr, "Fatal Error: %s\n", s);
    exit(1); // Exit the program with a non-zero status
}

// ---- optimizer.c Helpers ----

bool isConstant(const char *str)
{
    if (str == NULL || *str == '\0')
    {
        return false;
    }
    if (*str == '-')
        ++str;
    while (*str)
    {
        if (!isdigit((unsigned char)*str))
            return false;
        ++str;
    }
    return true;
}

bool isVariable(const char *str)
{
    if (str == NULL || *str == '\0')
        return false;
    if (!isalpha((unsigned char)*str) && *str != '_')
        return false;
    ++str;
    while (*str)
    {
        if (!isalnum((unsigned char)*str) && *str != '_')
            return false;
        ++str;
    }
    return true;
}

// ---- semantic.c Helpers ----

void printAllSymbolTables(SymbolTable *symbolTable)
{
    if (symbolTable == NULL)
    {
        printf("No symbol tables to print.\n");
        return;
    }

    printf("Printing All Symbol Tables:\n");
    printf("================================================================\n");

    // First, traverse backward to the outermost scope
    SymbolTable *currentTable = symbolTable;
    while (currentTable->prev != NULL)
    {
        currentTable = currentTable->prev;
    }

    // Now traverse forward to print all symbol tables
    while (currentTable != NULL)
    {
        printf("\n--- Symbol Table at Scope: %d ---\n", currentTable->scope);
        printSymbolTable(currentTable); // Use the simplified print function
        currentTable = currentTable->next; // Move to the next inner scope
    }

    printf("================================================================\n");
}

void printSymbolTable(SymbolTable *symbolTable)
{
    if (symbolTable == NULL)
    {
        printf("Symbol table is empty.\n");
        return;
    }

    printf("Symbol Table (Scope: %d):\n", symbolTable->scope);
    printf("================================================================\n");
    printf("| %-20s | %-10s | %-6s | %-5s | %-5s |\n", "Name", "Type", "Index", "Array", "Func");
    printf("================================================================\n");

    for (int i = 0; i < symbolTable->size; i++)
    {
        Symbol *symbol = symbolTable->table[i];
        while (symbol != NULL)
        {
            printf("| %-20s | %-10s | %-6d | %-5s | %-5s |\n",
                   symbol->name,
                   symbol->type,
                   symbol->index,
                   symbol->isArray ? "Yes" : "No",
                   symbol->isFunction ? "Yes" : "No");

            if (symbol->isArray)
            {
                printf("    [Array]\n");
            }

            if (symbol->isFunction && symbol->paramList != NULL)
            {
                printf("    [Function with parameters]\n");
            }

            symbol = symbol->next;
        }
    }

    printf("================================================================\n");

    // Recursively print inner scopes
    if (symbolTable->next != NULL)
    {
        printf("\nEntering Inner Scope:\n");
        printSymbolTable(symbolTable->next);
    }
}

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
                // print: arrayName [ index ] = rhs
                fprintf(file, "%s [ %s ] = %s\n", current->arg1 ? current->arg1 : "", current->arg2 ? current->arg2 : "", current->result ? current->result : "");
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

void printNodeDetails(ASTNode *node)
{
    if (!node)
    {
        return;
    }

    printf("Node Type: %s\n", nodeTypeNames[node->type]);

    switch (node->type)
    {
    case NodeType_SimpleExpr:
        if (node->simpleExpr.isFloat)
        {
            printf("Float Value: %f\n", node->simpleExpr.floatValue);
        }
        else
        {
            printf("Number: %d\n", node->simpleExpr.number);
        }
        break;

    case NodeType_SimpleID:
        printf("Name: %s\n", node->simpleID.name);
        break;

    case NodeType_FuncDecl:
        printf("Function Name: %s, Return Type: %s\n", node->funcDecl.funcName, node->funcDecl.returnType);
        break;

    case NodeType_Param:
        printf("Parameter Name: %s, Type: %s\n", node->param.paramName, node->param.paramType);
        break;

    case NodeType_VarDecl:
        printf("Variable Name: %s, Type: %s\n", node->varDecl.varName, node->varDecl.varType);
        break;

    case NodeType_ArrayDecl:
        printf("Array Name: %s, Type: %s, Size: %d\n", node->arrayDecl.varName, node->arrayDecl.varType, node->arrayDecl.size);
        break;

    case NodeType_AssignStmt:
        printf("Assign to Variable: %s\n", node->assignStmt.varName);
        break;

    default:
        // Handle other node types as needed
        break;
    }

    // Recursively traverse child nodes based on the node type
    if (node->type == NodeType_Program && node->program.declList)
    {
        traverseAST(node->program.declList, 1);
    }
    else if (node->type == NodeType_DeclList)
    {
        traverseAST(node->declList.decl, 1);
        traverseAST(node->declList.next, 1);
    }
    else if (node->type == NodeType_FuncDecl && node->funcDecl.block)
    {
        traverseAST(node->funcDecl.block, 1);
    }
    else if (node->type == NodeType_StmtList)
    {
        traverseAST(node->stmtList.stmt, 1);
        traverseAST(node->stmtList.stmtList, 1);
    }
}