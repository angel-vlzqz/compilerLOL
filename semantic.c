#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "SymbolTable.h"
#include "semantic.h"
#include "utils.h"
#include "temp.h"
#include "codeGenerator.h"

#define TABLE_SIZE 101
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
        printf("bussy 1\n");
        // Analyze the program
        semanticAnalysis(node->program.declList, symTab);
        break;

    case NodeType_DeclList:
        printf("bussy 2\n");
        // Analyze declarations
        semanticAnalysis(node->declList.decl, symTab);
        semanticAnalysis(node->declList.next, symTab);
        break;

    case NodeType_FuncDecl:
    {
        printf("bussy 3\n"); 
        // Use the symbol table for the function's scope
        SymbolTable *functionScope = getSymbolTableAtDepth(symTab, symTab->scope + 1);
        if (functionScope == NULL)
        {
            fprintf(stderr, "Semantic error: Function scope not found for function %s\n", node->funcDecl.funcName);
            exit(1);
        }

        // Analyze variable declarations
        if (node->funcDecl.varDeclList)
            semanticAnalysis(node->funcDecl.varDeclList, functionScope);

        // Analyze the function body
        if (node->funcDecl.block)
            semanticAnalysis(node->funcDecl.block, functionScope);

        // Analyze the return statement
        if (node->funcDecl.returnStmt)
            semanticAnalysis(node->funcDecl.returnStmt, functionScope);

        // Generate TAC for the function
        generateTACForFunction(node, functionScope);

        break;
    }

    case NodeType_VarDeclList:
        printf("bussy 4\n");
        semanticAnalysis(node->varDeclList.varDecl, symTab);
        semanticAnalysis(node->varDeclList.varDeclList, symTab);
        break;

    case NodeType_VarDecl:
        printf("penis 1\n");
        // Analyze the initial value if present
        if (node->varDecl.initialValue)
            semanticAnalysis(node->varDecl.initialValue, symTab);
        break;

    case NodeType_ArrayDecl:
        printf("penis 2\n");
        // No additional semantic analysis needed for array declarations
        break;

    case NodeType_StmtList:
        printf("penis 3\n");
        semanticAnalysis(node->stmtList.stmt, symTab);
        semanticAnalysis(node->stmtList.stmtList, symTab);
        break;

    case NodeType_AssignStmt:
        printf("penis 4\n");
        // Analyze the expression
        semanticAnalysis(node->assignStmt.expr, symTab);

        // Generate TAC for the assignment
        generateTACForExpr(node, symTab);

        break;

    case NodeType_BinOp:
        printf("penis 5\n");
        semanticAnalysis(node->binOp.left, symTab);
        semanticAnalysis(node->binOp.right, symTab);

        // Type checking and setting dataType
        if ((strcmp(node->binOp.left->dataType, "int") == 0 && strcmp(node->binOp.right->dataType, "float") == 0) ||
            (strcmp(node->binOp.left->dataType, "float") == 0 && strcmp(node->binOp.right->dataType, "int") == 0))
        {
            node->dataType = strdup("float"); // Promote to float
        }
        else if (strcmp(node->binOp.left->dataType, node->binOp.right->dataType) == 0)
        {
            node->dataType = strdup(node->binOp.left->dataType);
        }
        else
        {
            fprintf(stderr, "Semantic error: Type mismatch in binary operation\n");
            exit(1);
        }

        break;

    case NodeType_SimpleID:
    {
        printf("penis 6\n");
        Symbol *symbol = findSymbol(symTab, node->simpleID.name);
        if (symbol == NULL)
        {
            fprintf(stderr, "Semantic error: Variable %s has not been declared\n", node->simpleID.name);
            exit(1);
        }

        node->dataType = strdup(symbol->type);
        break;
    }

    case NodeType_SimpleExpr:
        printf("penis 7\n");
        node->dataType = node->simpleExpr.isFloat ? strdup("float") : strdup("int");
        break;

    case NodeType_WriteStmt:
        printf("penis 8\n");
        semanticAnalysis(node->writeStmt.expr, symTab);
        generateTACForExpr(node, symTab);
        break;

    case NodeType_Block:
        printf("penis 9\n");
        if (node->block.stmtList)
            semanticAnalysis(node->block.stmtList, symTab);
        break;

    case NodeType_ReturnStmt:
        printf("penis 10\n");
        if (node->returnStmt.expr)
            semanticAnalysis(node->returnStmt.expr, symTab);
        break;

    case NodeType_IfStmt:
        // Analyze the condition
        semanticAnalysis(node->ifStmt.condition, symTab);

        // Analyze the then block
        if (node->ifStmt.thenBlock)
            semanticAnalysis(node->ifStmt.thenBlock, symTab);

        // Analyze the else block, which can be another IfStmt or a Block
        if (node->ifStmt.elseBlock)
            semanticAnalysis(node->ifStmt.elseBlock, symTab);

        // Generate TAC for the if statement
        generateTACForExpr(node, symTab);
        break;

    case NodeType_WhileStmt:
        // Analyze the condition
        semanticAnalysis(node->whileStmt.condition, symTab);

        // Analyze the block
        if (node->whileStmt.block)
            semanticAnalysis(node->whileStmt.block, symTab);

        // Generate TAC for the while loop
        generateTACForExpr(node, symTab);
        break;

    case NodeType_FuncCall:
    {
        // Analyze arguments
        ASTNode *argNode = node->funcCall.argList;
        while (argNode)
        {
            semanticAnalysis(argNode->argList.arg, symTab);
            argNode = argNode->argList.argList;
        }

        // Generate TAC for the function call
        generateTACForExpr(node, symTab);
        break;
    }

    case NodeType_ArgList:
        semanticAnalysis(node->argList.arg, symTab);
        semanticAnalysis(node->argList.argList, symTab);
        break;

    case NodeType_Arg:
        semanticAnalysis(node->arg.expr, symTab);
        node->dataType = strdup(node->arg.expr->dataType);
        break;

    case NodeType_CastExpr:
        // Analyze the expression being cast
        semanticAnalysis(node->castExpr.expr, symTab);
        node->dataType = strdup(node->castExpr.type);
        break;

    case NodeType_LogicalOp:
        semanticAnalysis(node->logicalOp.left, symTab);
        semanticAnalysis(node->logicalOp.right, symTab);
        // Type checking can be added here
        node->dataType = strdup("bool");
        break;

    case NodeType_RelOp:
        semanticAnalysis(node->relOp.left, symTab);
        semanticAnalysis(node->relOp.right, symTab);
        // Type checking can be added here
        node->dataType = strdup("bool");
        break;

    case NodeType_NotOp:
        semanticAnalysis(node->notOp.expr, symTab);
        // Type checking can be added here
        node->dataType = strdup("bool");
        break;

        case NodeType_ArrayAssign:
        {
            // Analyze the index expression
            semanticAnalysis(node->arrayAssign.index, symTab);

            // Analyze the right-hand side expression
            semanticAnalysis(node->arrayAssign.expr, symTab);

            // Generate TAC for the array assignment
            generateTACForExpr(node, symTab);
            break;
        }

        case NodeType_ArrayAccess:
        {
            // Analyze the index expression
            semanticAnalysis(node->arrayAccess.index, symTab);

            // Set the data type based on the array's element type
            Symbol *symbol = findSymbol(symTab, node->arrayAccess.arrayName);
            if (symbol == NULL || !symbol->isArray)
            {
                fprintf(stderr, "Semantic error: Array %s has not been declared\n", node->arrayAccess.arrayName);
                exit(1);
            }

            node->dataType = strdup(symbol->type);
            break;
        }


    default:
        fprintf(stderr, "Semantic error: Unhandled node type %d\n", node->type);
        break;
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
        printf("bussy 5\n");
        char *rhs = generateTACForExpr(expr->assignStmt.expr, symTab);
        char *lhs = expr->assignStmt.varName;

        // Create TAC for assignment
        TAC *assignTAC = (TAC *)malloc(sizeof(TAC));
        assignTAC->op = strdup("=");
        assignTAC->arg1 = strdup(rhs);
        assignTAC->arg2 = NULL;
        assignTAC->result = strdup(lhs);
        assignTAC->next = NULL;

        appendTAC(&tacHead, assignTAC);

        return lhs;
    }

    case NodeType_BinOp:
    {
        printf("bussy 6\n");
        char *left = generateTACForExpr(expr->binOp.left, symTab);
        char *right = generateTACForExpr(expr->binOp.right, symTab);

        char *result = createTempVar(symTab);

        // Determine the operation
        char op[4];
        snprintf(op, sizeof(op), "%c", expr->binOp.operator);

        // Create TAC for binary operation
        TAC *binOpTAC = (TAC *)malloc(sizeof(TAC));
        binOpTAC->op = strdup(op);
        binOpTAC->arg1 = strdup(left);
        binOpTAC->arg2 = strdup(right);
        binOpTAC->result = strdup(result);
        binOpTAC->next = NULL;

        appendTAC(&tacHead, binOpTAC);

        return result;
    }

    case NodeType_SimpleExpr:
    {
        printf("bussy 7\n");
        char buffer[32];
        if (expr->simpleExpr.isFloat)
            snprintf(buffer, sizeof(buffer), "%.6f", expr->simpleExpr.floatValue);
        else
            snprintf(buffer, sizeof(buffer), "%d", expr->simpleExpr.number);

        return strdup(buffer);
    }

    case NodeType_SimpleID:
        printf("bussy 8\n");
        return strdup(expr->simpleID.name);

    case NodeType_WriteStmt:
    {
        printf("bussy 9\n");
        char *exprResult = generateTACForExpr(expr->writeStmt.expr, symTab);

        // Create TAC for write
        TAC *writeTAC = (TAC *)malloc(sizeof(TAC));
        writeTAC->op = strdup("write");
        writeTAC->arg1 = strdup(exprResult);
        writeTAC->arg2 = NULL;
        writeTAC->result = NULL;
        writeTAC->next = NULL;

        appendTAC(&tacHead, writeTAC);

        return NULL;
    }

    case NodeType_IfStmt:
    {
        printf("bussy 10\n");
        // Generate labels
        char *labelTrue = createLabel();
        char *labelFalse = createLabel();
        char *labelEnd = createLabel();

        // Generate TAC for condition
        char *condResult = generateTACForExpr(expr->ifStmt.condition, symTab);

        // Create TAC for conditional jump
        TAC *ifTAC = (TAC *)malloc(sizeof(TAC));
        ifTAC->op = strdup("ifFalse");
        ifTAC->arg1 = strdup(condResult);
        ifTAC->arg2 = NULL;
        ifTAC->result = strdup(labelFalse);
        ifTAC->next = NULL;

        appendTAC(&tacHead, ifTAC);

        // Then block
        generateTACForExpr(expr->ifStmt.thenBlock, symTab);

        // Jump to end
        TAC *gotoEndTAC = (TAC *)malloc(sizeof(TAC));
        gotoEndTAC->op = strdup("goto");
        gotoEndTAC->arg1 = strdup(labelEnd);
        gotoEndTAC->arg2 = NULL;
        gotoEndTAC->result = NULL;
        gotoEndTAC->next = NULL;

        appendTAC(&tacHead, gotoEndTAC);

        // False label
        TAC *labelFalseTAC = (TAC *)malloc(sizeof(TAC));
        labelFalseTAC->op = strdup("label");
        labelFalseTAC->arg1 = strdup(labelFalse);
        labelFalseTAC->arg2 = NULL;
        labelFalseTAC->result = NULL;
        labelFalseTAC->next = NULL;

        appendTAC(&tacHead, labelFalseTAC);

        // Else block if present
        if (expr->ifStmt.elseBlock)
            generateTACForExpr(expr->ifStmt.elseBlock, symTab);

        // End label
        TAC *labelEndTAC = (TAC *)malloc(sizeof(TAC));
        labelEndTAC->op = strdup("label");
        labelEndTAC->arg1 = strdup(labelEnd);
        labelEndTAC->arg2 = NULL;
        labelEndTAC->result = NULL;
        labelEndTAC->next = NULL;

        appendTAC(&tacHead, labelEndTAC);

        // Free labels
        free(labelTrue);
        free(labelFalse);
        free(labelEnd);

        return NULL;
    }

    case NodeType_WhileStmt:
    {
        printf("bussy 11\n");
        // Generate labels
        char *labelStart = createLabel();
        char *labelEnd = createLabel();

        // Start label
        TAC *labelStartTAC = (TAC *)malloc(sizeof(TAC));
        labelStartTAC->op = strdup("label");
        labelStartTAC->arg1 = strdup(labelStart);
        labelStartTAC->arg2 = NULL;
        labelStartTAC->result = NULL;
        labelStartTAC->next = NULL;

        appendTAC(&tacHead, labelStartTAC);

        // Generate TAC for condition
        char *condResult = generateTACForExpr(expr->whileStmt.condition, symTab);

        // Conditional jump to end
        TAC *ifFalseTAC = (TAC *)malloc(sizeof(TAC));
        ifFalseTAC->op = strdup("ifFalse");
        ifFalseTAC->arg1 = strdup(condResult);
        ifFalseTAC->arg2 = NULL;
        ifFalseTAC->result = strdup(labelEnd);
        ifFalseTAC->next = NULL;

        appendTAC(&tacHead, ifFalseTAC);

        // Loop body
        generateTACForExpr(expr->whileStmt.block, symTab);

        // Jump back to start
        TAC *gotoStartTAC = (TAC *)malloc(sizeof(TAC));
        gotoStartTAC->op = strdup("goto");
        gotoStartTAC->arg1 = strdup(labelStart);
        gotoStartTAC->arg2 = NULL;
        gotoStartTAC->result = NULL;
        gotoStartTAC->next = NULL;

        appendTAC(&tacHead, gotoStartTAC);

        // End label
        TAC *labelEndTAC = (TAC *)malloc(sizeof(TAC));
        labelEndTAC->op = strdup("label");
        labelEndTAC->arg1 = strdup(labelEnd);
        labelEndTAC->arg2 = NULL;
        labelEndTAC->result = NULL;
        labelEndTAC->next = NULL;

        appendTAC(&tacHead, labelEndTAC);

        // Free labels
        free(labelStart);
        free(labelEnd);

        return NULL;
    }

    case NodeType_FuncCall:
    {
        printf("bussy 12\n");
        // Process arguments
        ASTNode *argNode = expr->funcCall.argList;
        int argCount = 0;

        while (argNode)
        {
            char *argResult = generateTACForExpr(argNode->argList.arg, symTab);

            // Create TAC for pushing argument
            TAC *paramTAC = (TAC *)malloc(sizeof(TAC));
            paramTAC->op = strdup("param");
            paramTAC->arg1 = strdup(argResult);
            paramTAC->arg2 = NULL;
            paramTAC->result = NULL;
            paramTAC->next = NULL;

            appendTAC(&tacHead, paramTAC);

            argNode = argNode->argList.argList;
            argCount++;
        }

        // Create TAC for function call
        TAC *callTAC = (TAC *)malloc(sizeof(TAC));
        callTAC->op = strdup("call");
        callTAC->arg1 = strdup(expr->funcCall.funcName);
        callTAC->arg2 = NULL;
        callTAC->result = NULL;
        callTAC->next = NULL;

        appendTAC(&tacHead, callTAC);

        // If the function returns a value
        Symbol *funcSymbol = findSymbol(symTab, expr->funcCall.funcName);
        if (strcmp(funcSymbol->type, "void") != 0)
        {
            char *result = createTempVar(symTab);

            // Create TAC for retrieving return value
            TAC *retTAC = (TAC *)malloc(sizeof(TAC));
            retTAC->op = strdup("retrieve");
            retTAC->arg1 = NULL;
            retTAC->arg2 = NULL;
            retTAC->result = strdup(result);
            retTAC->next = NULL;

            appendTAC(&tacHead, retTAC);

            return result;
        }

        return NULL;
    }

    case NodeType_ReturnStmt:
    {
        printf("bussy 13\n");
        char *retValue = NULL;

        if (expr->returnStmt.expr)
            retValue = generateTACForExpr(expr->returnStmt.expr, symTab);

        // Create TAC for return
        TAC *returnTAC = (TAC *)malloc(sizeof(TAC));
        returnTAC->op = strdup("return");
        returnTAC->arg1 = retValue ? strdup(retValue) : NULL;
        returnTAC->arg2 = NULL;
        returnTAC->result = NULL;
        returnTAC->next = NULL;

        appendTAC(&tacHead, returnTAC);

        return NULL;
    }

    case NodeType_Block:
        printf("bussy 14\n");
        // Process statements in the block
        if (expr->block.stmtList)
            generateTACForExpr(expr->block.stmtList, symTab);
        return NULL;

    case NodeType_StmtList:
        printf("bussy 15\n");
        generateTACForExpr(expr->stmtList.stmt, symTab);
        generateTACForExpr(expr->stmtList.stmtList, symTab);
        return NULL;

    case NodeType_LogicalOp:
    {
        printf("bussy 16\n");
        char *left = generateTACForExpr(expr->logicalOp.left, symTab);
        char *right = generateTACForExpr(expr->logicalOp.right, symTab);
        char *result = createTempVar(symTab);

        // Create TAC for logical operation
        TAC *logicalTAC = (TAC *)malloc(sizeof(TAC));
        logicalTAC->op = strdup(expr->logicalOp.logicalOp); // "&&" or "||"
        logicalTAC->arg1 = strdup(left);
        logicalTAC->arg2 = strdup(right);
        logicalTAC->result = strdup(result);
        logicalTAC->next = NULL;

        appendTAC(&tacHead, logicalTAC);

        return result;
    }

    case NodeType_RelOp:
    {
        printf("bussy 17\n");
        char *left = generateTACForExpr(expr->relOp.left, symTab);
        char *right = generateTACForExpr(expr->relOp.right, symTab);
        char *result = createTempVar(symTab);

        // Create TAC for relational operation
        TAC *relOpTAC = (TAC *)malloc(sizeof(TAC));
        relOpTAC->op = strdup(expr->relOp.operator); // "==", "!=", "<", etc.
        relOpTAC->arg1 = strdup(left);
        relOpTAC->arg2 = strdup(right);
        relOpTAC->result = strdup(result);
        relOpTAC->next = NULL;

        appendTAC(&tacHead, relOpTAC);

        return result;
    }

    case NodeType_NotOp:
    {
        printf("bussy 18\n");
        char *exprResult = generateTACForExpr(expr->notOp.expr, symTab);
        char *result = createTempVar(symTab);

        // Create TAC for NOT operation
        TAC *notOpTAC = (TAC *)malloc(sizeof(TAC));
        notOpTAC->op = strdup("!");
        notOpTAC->arg1 = strdup(exprResult);
        notOpTAC->arg2 = NULL;
        notOpTAC->result = strdup(result);
        notOpTAC->next = NULL;

        appendTAC(&tacHead, notOpTAC);

        return result;
    }

    default:
        fprintf(stderr, "Error: Unsupported node type %d in TAC generation\n", expr->type);
        return NULL;
    }
}

void generateTACForFunction(ASTNode *funcNode, SymbolTable *symTab)
{
    if (!funcNode || !funcNode->funcDecl.funcName)
        return;

    // Function label
    TAC *labelTAC = (TAC *)malloc(sizeof(TAC));
    labelTAC->op = strdup("label");
    labelTAC->arg1 = strdup(funcNode->funcDecl.funcName);
    labelTAC->arg2 = NULL;
    labelTAC->result = NULL;
    labelTAC->next = NULL;

    appendTAC(&tacHead, labelTAC);

    // Prologue
    TAC *prologueTAC = (TAC *)malloc(sizeof(TAC));
    prologueTAC->op = strdup("prologue");
    prologueTAC->arg1 = strdup(funcNode->funcDecl.funcName);
    prologueTAC->arg2 = NULL;
    prologueTAC->result = NULL;
    prologueTAC->next = NULL;

    appendTAC(&tacHead, prologueTAC);

    // Generate TAC for function body
    if (funcNode->funcDecl.block)
        generateTACForExpr(funcNode->funcDecl.block, symTab);

    // Epilogue
    TAC *epilogueTAC = (TAC *)malloc(sizeof(TAC));
    epilogueTAC->op = strdup("epilogue");
    epilogueTAC->arg1 = strdup(funcNode->funcDecl.funcName);
    epilogueTAC->arg2 = NULL;
    epilogueTAC->result = NULL;
    epilogueTAC->next = NULL;

    appendTAC(&tacHead, epilogueTAC);
}

char *createTempVar(SymbolTable *symTab)
{
    static int tempVarCounter = 0;
    char *tempVarName = (char *)malloc(16);
    sprintf(tempVarName, "t%d", tempVarCounter++);

    // Insert temp variable into symbol table
    insertSymbol(symTab, tempVarName, "int", false, false, NULL);

    return tempVarName;
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

char *createLabel() 
{
    static int labelCounter = 0;
    char *label = (char *)malloc(16);
    sprintf(label, "L%d", labelCounter++);
    return label;
}
