#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "semantic.h"
#include "optimizer.h"
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
        // Check for valid
        if (strcmp(node->varDecl.varType, "int") == 0 ||
            strcmp(node->varDecl.varType, "float") == 0 ||
            strcmp(node->varDecl.varType, "char") == 0 ||
            strcmp(node->varDecl.varType, "bool") == 0 ||
            strcmp(node->varDecl.varType, "void") == 0)
        {
            // Valid type, proceed with the insertion into the symbol table
            if (findSymbol(symTab, node->varDecl.varName) != NULL)
            {
                fprintf(stderr, "Semantic error: Variable %s is already declared\n", node->varDecl.varName);
            }
            else
            {
                // For simple variable declarations, isArray is false, arrayInfo is NULL
                insertSymbol(symTab, node->varDecl.varName, node->varDecl.varType, false, NULL);
            }
        }
        else if (strcmp(getSymbolValue(symTab, node->varDecl.varName), "void") == 0)
        {
            fprintf(stderr, "Semantic error: Cannot assign value to variable of type void\n");
            exit(1);
        }
        else
        {
            fprintf(stderr, "Semantic error: Invalid type %s\n", node->varDecl.varType);
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
        // Ensure both operands are either int or float for arithmetic
        if ((strcmp(node->binOp.left->dataType, "int") == 0 || strcmp(node->binOp.left->dataType, "float") == 0) &&
            (strcmp(node->binOp.right->dataType, "int") == 0 || strcmp(node->binOp.right->dataType, "float") == 0))
        {
            // Allow the operation
            node->dataType = strdup("float"); // Handle implicit promotion to float if needed
        }
        else if (strcmp(node->binOp.left->dataType, node->binOp.right->dataType))
        {
            node->dataType = strdup(node->binOp.left->dataType);
        }
        else
        {
            fprintf(stderr, "Semantic error: Invalid operand types for binary operation\n");
            exit(1);
        }
        break;

    case NodeType_SimpleID:
    {
        Symbol *symbol = findSymbol(symTab, node->simpleID.name);
        if (symbol == NULL)
        {
            fprintf(stderr, "Semantic error: Variable %s has not been declared\n", node->simpleID.name);
        }
        else
        {
            // Set dataType based on symbol's type
            node->dataType = strdup(symbol->type);
        }
        break;
    }

    case NodeType_SimpleExpr:
        node->dataType = strdup("int");
        break;

    case NodeType_WriteStmt:
        semanticAnalysis(node->writeStmt.expr, symTab);
        // Generate TAC for the write statement
        generateTACForExpr(node, symTab);
        break;

    case NodeType_Block:
        if (node->block.stmtList != NULL)
            semanticAnalysis(node->block.stmtList, symTab);
        break;

    case NodeType_ArrayDecl:
    {
        // In NodeType_VarDecl or NodeType_ArrayDecl
        if (strcmp(node->varDecl.varType, "int") == 0 ||
            strcmp(node->varDecl.varType, "float") == 0 ||
            strcmp(node->varDecl.varType, "char") == 0 ||
            strcmp(node->varDecl.varType, "bool") == 0 ||
            strcmp(node->varDecl.varType, "void") == 0)
        {
            // Valid type, proceed with the insertion into the symbol table
            // Check for duplicate declaration
            if (findSymbol(symTab, node->arrayDecl.varName) != NULL)
            {
                fprintf(stderr, "Semantic error: Array %s is already declared\n", node->arrayDecl.varName);
                exit(1);
            }
            // Create array info
            Array *arrayInfo = createArray(node->arrayDecl.varType, node->arrayDecl.size);
            // Insert into symbol table
            insertSymbol(symTab, node->arrayDecl.varName, node->arrayDecl.varType, true, arrayInfo);
        }
        else
        {
            fprintf(stderr, "Semantic error: Invalid type %s\n", node->varDecl.varType);
        }
        break;
    }

    case NodeType_ArrayAssign:
    {
        Symbol *arraySymbol = findSymbol(symTab, node->arrayAssign.arrayName);
        if (arraySymbol == NULL || !arraySymbol->isArray)
        {
            fprintf(stderr, "Semantic error: %s is not a declared array\n", node->arrayAssign.arrayName);
            exit(1);
        }

        // Analyze index and expression
        semanticAnalysis(node->arrayAssign.index, symTab);
        semanticAnalysis(node->arrayAssign.expr, symTab);

        // Type checks
        if (strcmp(node->arrayAssign.index->dataType, "int") != 0)
        {
            fprintf(stderr, "Semantic error: Array index must be an integer\n");
            exit(1);
        }

        if (strcmp(node->arrayAssign.expr->dataType, arraySymbol->type) != 0)
        {
            fprintf(stderr, "Semantic error: Type mismatch in array assignment\n");
            exit(1);
        }

        // Generate TAC for the array assignment
        generateTACForExpr(node, symTab);

        break;
    }

    case NodeType_ArrayAccess:
    {
        Symbol *arraySymbol = findSymbol(symTab, node->arrayAccess.arrayName);
        if (arraySymbol == NULL || !arraySymbol->isArray)
        {
            fprintf(stderr, "Semantic error: %s is not a declared array\n", node->arrayAccess.arrayName);
            exit(1);
        }

        // Analyze index
        semanticAnalysis(node->arrayAccess.index, symTab);

        // Type checks
        if (strcmp(node->arrayAccess.index->dataType, "int") != 0)
        {
            fprintf(stderr, "Semantic error: Array index must be an integer\n");
            exit(1);
        }

        // Set the data type of the array access node
        node->dataType = strdup(arraySymbol->type);

        // Generate TAC for the array access
        generateTACForExpr(node, symTab);

        break;
    }

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
        char opStr[2] = {expr->binOp.operator, '\0' };
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

        // Return the constant as a string
        return strdup(buffer);
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
        // Generate TAC for the expression to write
        char *exprResult = generateTACForExpr(expr->writeStmt.expr, symTab);

        // Create a TAC instruction for the write operation
        TAC *writeTAC = (TAC *)malloc(sizeof(TAC));
        // Check if exprResult is a constant
        if (isConstant(exprResult))
        {
            // Handle constant write
            writeTAC->op = strdup("write");
            writeTAC->arg1 = strdup(exprResult);
        }
        else
        {
            Symbol *foundSymbol = findSymbol(symTab, exprResult);
            if (foundSymbol == NULL)
            {
                fprintf(stderr, "Error: Symbol '%s' not found in the symbol table.\n", exprResult);
                return NULL; // Handle the error gracefully
            }

            writeTAC->op = strdup("write");
            writeTAC->arg1 = strdup(exprResult);
        }

        // Common TAC fields
        writeTAC->arg2 = NULL;
        writeTAC->result = NULL;
        writeTAC->next = NULL;

        appendTAC(&tacHead, writeTAC);
        return NULL;
    }
    break;

    case NodeType_ArrayAssign:
    {
        // Generate TAC for index and expression
        char *index = generateTACForExpr(expr->arrayAssign.index, symTab);
        char *rhs = generateTACForExpr(expr->arrayAssign.expr, symTab);

        // Create a TAC instruction for the array assignment
        TAC *arrayAssignTAC = (TAC *)malloc(sizeof(TAC));
        arrayAssignTAC->op = strdup("[]=");
        arrayAssignTAC->arg1 = strdup(index);                         // This holds the index
        arrayAssignTAC->arg2 = strdup(rhs);                           // This holds the value being assigned
        arrayAssignTAC->result = strdup(expr->arrayAssign.arrayName); // This holds the array name
        arrayAssignTAC->next = NULL;

        appendTAC(&tacHead, arrayAssignTAC);
        return NULL;
    }
    break;

    case NodeType_ArrayAccess:
    {
        // Generate TAC for the index
        char *index = generateTACForExpr(expr->arrayAccess.index, symTab);

        // Create a TAC instruction for the array access
        TAC *arrayAccessTAC = (TAC *)malloc(sizeof(TAC));
        arrayAccessTAC->op = strdup("=[]");
        arrayAccessTAC->arg1 = strdup(expr->arrayAccess.arrayName); // This holds the array name
        arrayAccessTAC->arg2 = strdup(index);                       // This holds the index
        arrayAccessTAC->result = createTempVar();                   // Create a temporary variable to hold the result
        arrayAccessTAC->next = NULL;

        appendTAC(&tacHead, arrayAccessTAC);
        return strdup(arrayAccessTAC->result); // Return the temporary variable
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

void freeTACList(TAC *head)
{
    TAC *current = head;
    while (current != NULL)
    {
        TAC *next = current->next;
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
            else if (strcmp(current->op, "[]=") == 0)
            {
                fprintf(file, "%s [ %s ] = %s\n", current->result, current->arg1, current->arg2);
            }
            else if (strcmp(current->op, "=[]") == 0)
            {
                fprintf(file, "%s = %s [ %s ]\n", current->result, current->arg1, current->arg2);
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
