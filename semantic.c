#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "semantic.h"
#include "temp.h"

int tempVars[50] = {0}; // Definition and initialization

// Global head of the TAC instructions list
TAC *tacHead = NULL;

void semanticAnalysis(ASTNode *node, SymbolTable *symTab)
{
    if (node == NULL)
        return;

    switch (node->type)
    {
    case NodeType_Program:
        semanticAnalysis(node->program.varDeclList, symTab);
        semanticAnalysis(node->program.block, symTab);
        break;

    case NodeType_VarDeclList:
        semanticAnalysis(node->varDeclList.varDecl, symTab);
        semanticAnalysis(node->varDeclList.varDeclList, symTab);
        break;

    case NodeType_VarDecl:
        if (findSymbol(symTab, node->varDecl.varName) != NULL)
        {
            fprintf(stderr, "Semantic error: Variable %s is already declared\n", node->varDecl.varName);
        }
        else
        {
            insertSymbol(symTab, node->varDecl.varName, node->varDecl.varType);
        }
        break;

    case NodeType_StmtList:
        if (node->stmtList.stmt != NULL)
            semanticAnalysis(node->stmtList.stmt, symTab);
        if (node->stmtList.stmtList != NULL)
            semanticAnalysis(node->stmtList.stmtList, symTab);
        break;

    case NodeType_AssignStmt:
        if (findSymbol(symTab, node->assignStmt.varName) == NULL)
        {
            fprintf(stderr, "Semantic error: Variable %s has not been declared\n", node->assignStmt.varName);
        }
        else
        {
            // Perform semantic analysis on the expression
            semanticAnalysis(node->assignStmt.expr, symTab);
            // Generate TAC for the assignment
            generateTACForExpr(node, symTab);
        }
        break;

    case NodeType_BinOp:
        semanticAnalysis(node->binOp.left, symTab);
        semanticAnalysis(node->binOp.right, symTab);
        // No need to generate TAC here; it's handled in `generateTACForExpr`
        break;

    case NodeType_SimpleID:
        if (findSymbol(symTab, node->simpleID.name) == NULL)
        {
            fprintf(stderr, "Semantic error: Variable %s has not been declared\n", node->simpleID.name);
        }
        break;

    case NodeType_SimpleExpr:
        // No action needed for simple expressions
        break;

    case NodeType_WriteStmt:
        if (findSymbol(symTab, node->writeStmt.varName) == NULL)
        {
            fprintf(stderr, "Semantic error: Variable %s has not been declared\n", node->writeStmt.varName);
        }
        else
        {
            // Generate TAC for the write statement
            generateTACForExpr(node, symTab);
        }
        break;

    case NodeType_Block:
        if (node->block.stmtList != NULL)
            semanticAnalysis(node->block.stmtList, symTab);
        break;

    default:
        fprintf(stderr, "Unknown Node Type: %d\n", node->type);
    }
}

char *generateTACForExpr(ASTNode *expr, SymbolTable *symTab)
{
    if (!expr)
        return NULL;

    switch (expr->type)
    {
    case NodeType_AssignStmt:
    {
        // Generate TAC for the right-hand side expression
        char *rhs = generateTACForExpr(expr->assignStmt.expr, symTab);

        // Create a TAC instruction for the assignment
        TAC *assignTAC = (TAC *)malloc(sizeof(TAC));
        assignTAC->op = strdup("=");
        assignTAC->arg1 = strdup(rhs);
        assignTAC->arg2 = NULL;
        assignTAC->result = strdup(expr->assignStmt.varName);
        assignTAC->next = NULL;

        appendTAC(&tacHead, assignTAC);
        return strdup(expr->assignStmt.varName);
    }
    break;

    case NodeType_BinOp:
    {
        // Generate TAC for left and right operands
        char *left = generateTACForExpr(expr->binOp.left, symTab);
        char *right = generateTACForExpr(expr->binOp.right, symTab);

        // Create a new TAC instruction for the binary operation
        TAC *binOpTAC = (TAC *)malloc(sizeof(TAC));
        char opStr[2] = {expr->binOp.operator, '\0'};
        binOpTAC->op = strdup(opStr);
        binOpTAC->arg1 = strdup(left);
        binOpTAC->arg2 = strdup(right);
        binOpTAC->result = createTempVar();
        binOpTAC->next = NULL;

        appendTAC(&tacHead, binOpTAC);
        return strdup(binOpTAC->result);
    }
    break;

    case NodeType_SimpleExpr:
    {
        // Numeric literal
        char buffer[20];
        snprintf(buffer, 20, "%d", expr->simpleExpr.number);

        // Create a TAC instruction to load the constant
        TAC *constTAC = (TAC *)malloc(sizeof(TAC));
        constTAC->op = strdup("=");
        constTAC->arg1 = strdup(buffer);
        constTAC->arg2 = NULL;
        constTAC->result = createTempVar();
        constTAC->next = NULL;

        appendTAC(&tacHead, constTAC);
        return strdup(constTAC->result);
    }
    break;

    case NodeType_SimpleID:
    {
        // Return the variable name
        return strdup(expr->simpleID.name);
    }
    break;

    case NodeType_WriteStmt:
    {
        // Create a TAC instruction for the write operation
        TAC *writeTAC = (TAC *)malloc(sizeof(TAC));
        writeTAC->op = strdup("write");
        writeTAC->arg1 = strdup(expr->writeStmt.varName);
        writeTAC->arg2 = NULL;
        writeTAC->result = NULL;
        writeTAC->next = NULL;

        appendTAC(&tacHead, writeTAC);
        return NULL;
    }
    break;

    default:
        fprintf(stderr, "Error: Unsupported node type %d in TAC generation\n", expr->type);
        return NULL;
    }
}

// Function to create a new temporary variable for TAC
char *createTempVar()
{
    int index = allocateNextAvailableTempVar(tempVars);
    if (index == -1)
    {
        fprintf(stderr, "Error: No available temporary variables\n");
        return NULL;
    }

    char *tempVar = malloc(10); // Enough space for "t" + number
    if (!tempVar)
        return NULL;

    sprintf(tempVar, "t%d", index);
    return tempVar;
}

void appendTAC(TAC **head, TAC *newInstruction)
{
    if (!*head)
    {
        *head = newInstruction;
    }
    else
    {
        TAC *current = *head;
        while (current->next)
        {
            current = current->next;
        }
        current->next = newInstruction;
    }
}

void freeTACList(TAC* head) 
{
    TAC* current = head;
    while (current != NULL) 
    {
        TAC* next = current->next;
        if (current->op)
            free(current->op);
        if (current->arg1)
            free(current->arg1);
        if (current->arg2)
            free(current->arg2);
        if (current->result)
            free(current->result);
        free(current);
        current = next;
    }
}

// Temporary variable allocation and deallocation functions

void initializeTempVars()
{
    for (int i = 0; i < 50; i++)
    {
        tempVars[i] = 0;
    }
}

int allocateNextAvailableTempVar(int tempVars[])
{
    for (int i = 0; i < 50; i++)
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
                fprintf(file, "%s = %s\n", current->result, current->arg1);
            }
            else if (strcmp(current->op, "write") == 0)
            {
                fprintf(file, "write %s\n", current->arg1);
            }
            else
            {
                fprintf(file, "%s = %s %s %s\n", current->result, current->arg1, current->op, current->arg2);
            }
        }
        current = current->next;
    }

    fclose(file);
}
