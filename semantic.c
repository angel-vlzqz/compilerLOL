#include <stdio.h>
#include <string.h>
#include "semantic.h"
#include "temp.h"

int tempVars[50] = {0}; // Definition and initialization

// Perform semantic analysis on the AST
TAC *tacHead = NULL;

void semanticAnalysis(ASTNode *node, SymbolTable *symTab)
{
    printf("====================================================================\n");
    if (node == NULL)
        return;

    switch (node->type)
    {
    case NodeType_Program:
        printf("Performing semantic analysis on program\n");
        semanticAnalysis(node->program.varDeclList, symTab);
        semanticAnalysis(node->program.block, symTab);
        fprintf(stderr, "Node Type: %d\n", node->type);
        break;
    case NodeType_VarDeclList:
        semanticAnalysis(node->varDeclList.varDecl, symTab);
        semanticAnalysis(node->varDeclList.varDeclList, symTab);
        fprintf(stderr, "Node Type: %d\n", node->type);
        break;
    case NodeType_VarDecl:
        // Check for redeclaration of variables
        if (findSymbol(symTab, node->varDecl.varName) != NULL)
        {
            fprintf(stderr, "Semantic error: Variable %s is already declared\n", node->varDecl.varName);
        }
        else
        {
            insertSymbol(symTab, node->varDecl.varName, node->varDecl.varType);
        }
        fprintf(stderr, "Node Type: %d\n", node->type);
        break;
    case NodeType_StmtList:
        semanticAnalysis(node->stmtList.stmt, symTab);
        semanticAnalysis(node->stmtList.stmtList, symTab);
        fprintf(stderr, "Node Type: %d\n", node->type);
        break;
    case NodeType_AssignStmt:
        if (findSymbol(symTab, node->assignStmt.varName) == NULL)
        {
            fprintf(stderr, "Semantic error: Variable %s has not been declared\n", node->assignStmt.varName);
        }
        semanticAnalysis(node->assignStmt.expr, symTab);
        fprintf(stderr, "Node Type: %d\n", node->type);

        // Generate TAC for the assignment statement
        generateTACForExpr(node, symTab);
        break;
    case NodeType_Expr:
        semanticAnalysis(node->expr.left, symTab);
        semanticAnalysis(node->expr.right, symTab);
        fprintf(stderr, "Node Type: %d\n", node->type);
        break;
    case NodeType_BinOp:
        // Check for variable declarations in binary operations
        if (findSymbol(symTab, node->binOp.left->varDecl.varName) == NULL)
        {
            fprintf(stderr, "Semantic error: Variable %s has not been declared\n", node->binOp.left->varDecl.varName);
        }
        if (findSymbol(symTab, node->binOp.right->varDecl.varName) == NULL)
        {
            fprintf(stderr, "Semantic error: Variable %s has not been declared\n", node->binOp.right->varDecl.varName);
        }
        semanticAnalysis(node->binOp.left, symTab);
        semanticAnalysis(node->binOp.right, symTab);
        fprintf(stderr, "Node Type: %d\n", node->type);

        // Generate TAC for the binary operation
        generateTACForExpr(node, symTab);
        break;
    case NodeType_LogicalOp:
        // Check logical operations
        semanticAnalysis(node->logicalOp.left, symTab);
        semanticAnalysis(node->logicalOp.right, symTab);
        fprintf(stderr, "Node Type: %d\n", node->type);
        break;
    case NodeType_IfStmt:
        semanticAnalysis(node->ifStmt.condition, symTab);
        semanticAnalysis(node->ifStmt.thenBlock, symTab);
        if (node->ifStmt.elseBlock != NULL)
        {
            semanticAnalysis(node->ifStmt.elseBlock, symTab);
        }
        fprintf(stderr, "Node Type: %d\n", node->type);
        break;
    case NodeType_WhileStmt:
        semanticAnalysis(node->whileStmt.condition, symTab);
        semanticAnalysis(node->whileStmt.block, symTab);
        fprintf(stderr, "Node Type: %d\n", node->type);
        break;
    case NodeType_ReturnStmt:
        semanticAnalysis(node->returnStmt.expr, symTab);
        fprintf(stderr, "Node Type: %d\n", node->type);
        break;
    case NodeType_SimpleID:
        if (findSymbol(symTab, node->simpleID.name) == NULL)
        {
            fprintf(stderr, "Semantic error: Variable %s has not been declared\n", node->simpleID.name);
            fprintf(stderr, "Node Type: %d\n", node->type);
        }
        break;
    case NodeType_SimpleExpr:
        // No checks necessary for numeric expressions
        break;
    case NodeType_WriteStmt:
        if (findSymbol(symTab, node->writeStmt.varName) == NULL)
        {
            fprintf(stderr, "Semantic error: Variable %s has not been declared\n", node->writeStmt.varName);
            fprintf(stderr, "Node Type: %d\n", node->type);
        }
        else
        {
            // Generate TAC for the write statement
            generateTACForExpr(node, symTab);
        }
        break;
    case NodeType_Block:
        printf("Performing semantic analysis on block\n");
        semanticAnalysis(node->block.stmtList, symTab);
        fprintf(stderr, "Node Type: %d\n", node->type);
        break;
    break;
    // ... handle other node types ...

    default:
        fprintf(stderr, "Unknown Node Type\n");
        fprintf(stderr, "Node Type: %d\n", node->type);
    }

    // ... other code ...

    if (node->type == NodeType_Expr || node->type == NodeType_SimpleExpr)
    {
        TAC *tac = generateTACForExpr(node, symTab);
        // Process or store the generated TAC
        printTAC(tac);
    }

    // ... other code ...
}

// You can add more functions related to semantic analysis here
// Implement functions to generate TAC expressions

TAC *generateTACForExpr(ASTNode *expr, SymbolTable *symTab)
{
    // Depending on your AST structure, generate the appropriate TAC
    // If the TAC is generated successfully, append it to the global TAC list
    // Return the generated TAC, so that it can be used by the caller, e.g. for printing
    if (!expr)
        return NULL;

    TAC *instruction = (TAC *)malloc(sizeof(TAC));
    if (!instruction)
    {
        perror("Failed to allocate memory for TAC");
        return NULL;
    }

    // Initialize TAC fields
    instruction->op = NULL;
    instruction->arg1 = NULL;
    instruction->arg2 = NULL;
    instruction->result = NULL;
    instruction->next = NULL;

    switch (expr->type)
    {
        case NodeType_BinOp:
        {
            TAC *leftTAC = generateTACForExpr(expr->binOp.left, symTab);
            TAC *rightTAC = generateTACForExpr(expr->binOp.right, symTab);

            if (leftTAC == NULL || rightTAC == NULL || leftTAC->result == NULL || rightTAC->result == NULL) {
                fprintf(stderr, "Error: Invalid operands for binary operation\n");
                free(instruction);
                return NULL;
            }

            instruction->op = strdup(&expr->binOp.operator); // e.g., "+", "-", etc.
            instruction->arg1 = strdup(leftTAC->result);
            instruction->arg2 = strdup(rightTAC->result);
            instruction->result = createTempVar(); // Allocate a new temporary variable for the result

            appendTAC(&tacHead, instruction);
            return instruction;
        }
        break;

        case NodeType_SimpleExpr:
        {
            // Handle numeric literals
            TAC *instruction = (TAC *)malloc(sizeof(TAC));
            instruction->op = strdup("="); // Assign operation
            instruction->arg1 = createOperand(expr, symTab); // Should return the numeric value as string
            instruction->arg2 = NULL;
            instruction->result = createTempVar();
            instruction->next = NULL;

            appendTAC(&tacHead, instruction);

            return instruction;
        }
        break;

        case NodeType_SimpleID:
        {
            // Handle variable identifiers
            // No need to generate a TAC instruction, just return the variable name
            TAC *instruction = (TAC *)malloc(sizeof(TAC));
            instruction->op = NULL;
            instruction->arg1 = NULL;
            instruction->arg2 = NULL;
            instruction->result = strdup(expr->simpleID.name);
            instruction->next = NULL;

            return instruction;
        }
        break;

        case NodeType_AssignStmt:
        {
            printf("Generating TAC for assignment statement\n");
            Symbol *assignSymbol = findSymbol(symTab, expr->assignStmt.varName);
            if (assignSymbol)
            {
                // Generate TAC for assigning an expression to a variable
                instruction->arg1 = createOperand(expr->assignStmt.expr, symTab); // The value to assign
                instruction->op = strdup("="); // Assignment operation
                instruction->result = strdup(expr->assignStmt.varName); // The variable being assigned to
            }
            else
            {
                fprintf(stderr, "Error: Variable %s has not been declared\n", expr->assignStmt.varName);
                free(instruction);
                return NULL;
            }
        }
        break;

        case NodeType_WriteStmt:
        {
            printf("Generating TAC for write statement\n");

            // Create a TAC instruction for the write operation
            TAC *writeTAC = (TAC *)malloc(sizeof(TAC));
            if (!writeTAC)
            {
                fprintf(stderr, "Failed to allocate memory for write TAC\n");
                return NULL;
            }

            writeTAC->op = strdup("write");
            writeTAC->arg1 = strdup(expr->writeStmt.varName); // Ensure this is valid
            writeTAC->arg2 = NULL;  // No second argument for write
            writeTAC->result = NULL; // No result for write
            writeTAC->next = NULL;

            appendTAC(&tacHead, writeTAC);
            return writeTAC;
        }
        break;

        default:
            fprintf(stderr, "Error: Unsupported node type in TAC generation\n");
            free(instruction);
            return NULL;
    }

    appendTAC(&tacHead, instruction);

    return instruction;
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

char *createOperand(ASTNode *node, SymbolTable *symTab)
{
    if (!node)
        return NULL;

    switch (node->type)
    {
        case NodeType_SimpleExpr:
        {
            // Numeric literal
            char buffer[20];
            snprintf(buffer, 20, "%d", node->simpleExpr.number);
            return strdup(buffer);
        }
        case NodeType_SimpleID:
        {
            // Variable identifier
            Symbol *symbol = findSymbol(symTab, node->simpleID.name);
            if (symbol)
                return strdup(node->simpleID.name);
            else
            {
                fprintf(stderr, "Error: Variable %s has not been declared\n", node->simpleID.name);
                return NULL;
            }
        }
        default:
            fprintf(stderr, "Unhandled node type %d in createOperand\n", node->type);
            return NULL;
    }

}

void printTAC(TAC *tac)
{
    if (!tac)
        return;

    // Print the TAC instruction with non-null fields
    if (tac->result != NULL)
        printf("%s = ", tac->result);
    if (tac->arg1 != NULL)
        printf("%s ", tac->arg1);
    if (tac->op != NULL)
        printf("%s ", tac->op);
    if (tac->arg2 != NULL)
        printf("%s ", tac->arg2);
    printf("\n");
}

// Print the TAC list to a file
// This function is provided for reference, you can modify it as needed

void printTACToFile(const char *filename, TAC *tac)
{
    printf("Opening file: %s\n", filename);
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        perror("Failed to open file");
        return;
    }

    TAC *current = tac;
    int tacCounter = 0;

    while (current != NULL)
    {
        printf("------------------------------------------------------------------------------\n");
        printf("Processing TAC instruction %d\n", tacCounter);

        if (current->op != NULL)
        {
            printf("TAC instruction %d: operation is: %s\n", tacCounter, current->op);
        }
        else
        {
            printf("TAC instruction %d: operation is NULL\n", tacCounter);
            current = current->next;
            tacCounter++;
            continue;
        }

        if (strcmp(current->op, "=") == 0)
        {
            if (current->result != NULL && current->arg1 != NULL)
            {
                fprintf(file, "%s = %s\n", current->result, current->arg1);
            }
            else
            {
                printf("TAC instruction %d: result or arg1 is NULL\n", tacCounter);
            }
        }
        else
        {
            // Check before printing 'result'
            if (current->result != NULL)
            {
                fprintf(file, "%s = ", current->result);
            }

            // Print 'arg1' safely
            if (current->arg1 != NULL)
            {
                fprintf(file, "%s ", current->arg1);
            }

            // Print 'op' safely
            if (current->op != NULL)
            {
                fprintf(file, "%s ", current->op);
            }

            // Print 'arg2' safely
            if (current->arg2 != NULL)
            {
                fprintf(file, "%s ", current->arg2);
            }

            fprintf(file, "\n");
        }

        current = current->next;
        tacCounter++;
    }

    printf("Closing file: %s\n", filename);
    fclose(file);
    printf("TAC written to %s\n", filename);
}
// Temporary variable allocation and deallocation functions //

void initializeTempVars()
{
    for (int i = 0; i < 50; i++)
    {
        tempVars[i] = 0;
    }
}

int allocateNextAvailableTempVar(int tempVars[])
{
    // implement the temp var allocation logic
    // use the tempVars array to keep track of allocated temp vars

    // search for the next available temp var
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
    // implement the temp var deallocation logic
    // use the tempVars array to keep track of allocated temp vars
    if (index >= 0 && index < 50)
    {
        tempVars[index] = 0;
    }
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