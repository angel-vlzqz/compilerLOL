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
    if (node)
    {
        printNodeDetails(node);
    }
    else
    {
        printf("Encountered NULL node\n");
    }
    if (node == NULL)
        return;

    printf("Visiting node of type: %d\n", node->type);

    switch (node->type)
    {
    case NodeType_Program:
        printf("Processing Program Node\n");
        // Analyze the program
        semanticAnalysis(node->program.declList, symTab);
        break;

    case NodeType_DeclList:
        printf("Processing DeclList Node at address %p\n", node);
        // Analyze declarations
        semanticAnalysis(node->declList.decl, symTab);
        if (node->declList.next)
        {
            printf("Moving to next DeclList Node at address %p\n", node->declList.next);
            semanticAnalysis(node->declList.next, symTab);
        }
        else
        {
            printf("No more DeclList Nodes\n");
        }
        break;

    case NodeType_FuncDecl: {
        printf("Processing FuncDecl Node: %s\n", node->funcDecl.funcName);

        // Create a new symbol table for the function if not already present
        SymbolTable *functionScope = node->funcDecl.symTab;
        if (functionScope == NULL) {
            functionScope = createSymbolTable(TABLE_SIZE, symTab);
            node->funcDecl.symTab = functionScope;
        }

        // Add the function to the global symbol table
        Symbol *funcSymbol = findSymbolInCurrentScope(symTab, node->funcDecl.funcName);
        if (funcSymbol == NULL) {
            insertSymbol(symTab, node->funcDecl.funcName, node->funcDecl.returnType, false, true, NULL);
            funcSymbol = findSymbolInCurrentScope(symTab, node->funcDecl.funcName);
            if (funcSymbol == NULL) {
                fprintf(stderr, "Error: Failed to insert function %s into symbol table\n", node->funcDecl.funcName);
                exit(1);
            }
            setSymbolParamList(funcSymbol, node->funcDecl.paramList);
        } else {
            fprintf(stderr, "Semantic error: Function %s is already declared\n", node->funcDecl.funcName);
            exit(1);
        }

        // Add parameters to the function's symbol table
        ASTNode *paramListNode = node->funcDecl.paramList;
        while (paramListNode) {
            ASTNode *paramNode = paramListNode->paramList.param;
            if (paramNode->type == NodeType_Param) {
                Symbol *existingParam = findSymbolInCurrentScope(functionScope, paramNode->param.paramName);
                if (existingParam != NULL) {
                    fprintf(stderr, "Semantic error: Parameter %s is already declared in function %s\n",
                            paramNode->param.paramName, node->funcDecl.funcName);
                    exit(1);
                }

                insertSymbol(functionScope, paramNode->param.paramName, paramNode->param.paramType, false, false, NULL);
                printf("Inserted parameter: %s of type %s into function %s\n",
                    paramNode->param.paramName, paramNode->param.paramType, node->funcDecl.funcName);
            }
            paramListNode = paramListNode->paramList.nextParam;
        }

        // Analyze local variable declarations
        semanticAnalysis(node->funcDecl.varDeclList, functionScope);

        // Analyze the function body
        semanticAnalysis(node->funcDecl.block, functionScope);

        // Analyze the return statement
        semanticAnalysis(node->funcDecl.returnStmt, functionScope);

        break;
    }

    case NodeType_VarDeclList:
        printf("Processing VarDeclList Node\n");
        semanticAnalysis(node->varDeclList.varDecl, symTab);
        semanticAnalysis(node->varDeclList.varDeclList, symTab);
        break;

    case NodeType_ParamList:
    {
        printf("Processing ParamList Node\n");
        semanticAnalysis(node->paramList.param, symTab);
        semanticAnalysis(node->paramList.nextParam, symTab);
        break;
    }

    case NodeType_Param:
    {
        printf("Processing Param Node: %s\n", node->param.paramName);

        // Check if the parameter is already declared in the current scope
        Symbol *existingSymbol = findSymbolInCurrentScope(symTab, node->param.paramName);
        if (existingSymbol != NULL)
        {
            fprintf(stderr, "Semantic error: Parameter %s is already declared in this scope\n", node->param.paramName);
            exit(1);
        }

        // Insert the parameter into the current scope's symbol table
        insertSymbol(symTab, node->param.paramName, node->param.paramType, false, false, NULL);

        break;
    }

    case NodeType_VarDecl: {
        printf("Processing VarDecl Node: %s\n", node->varDecl.varName);

        // Add the variable to the current scope
        Symbol *existingSymbol = findSymbolInCurrentScope(symTab, node->varDecl.varName);
        if (existingSymbol != NULL) {
            fprintf(stderr, "Semantic error: Variable %s is already declared in this scope\n", node->varDecl.varName);
            exit(1);
        }

        insertSymbol(symTab, node->varDecl.varName, node->varDecl.varType, false, false, NULL);
        break;
    }

    case NodeType_ArrayDecl:
    {
        printf("Processing ArrayDecl Node: %s\n", node->arrayDecl.varName);

        // Check if the array is already declared in the current scope
        Symbol *existingSymbol = findSymbolInCurrentScope(symTab, node->arrayDecl.varName);
        if (existingSymbol != NULL)
        {
            fprintf(stderr, "Semantic error: Array %s is already declared in this scope\n", node->arrayDecl.varName);
            exit(1);
        }

        // Create array information
        Array *arrayInfo = createArray(node->arrayDecl.varType, node->arrayDecl.size);

        // Insert the array into the symbol table
        insertSymbol(symTab, node->arrayDecl.varName, node->arrayDecl.varType, true, false, arrayInfo);

        break;
    }

    case NodeType_StmtList:
        printf("Processing StmtList Node\n");
        semanticAnalysis(node->stmtList.stmt, symTab);
        semanticAnalysis(node->stmtList.stmtList, symTab);
        break;

    case NodeType_AssignStmt:
    {
        printf("Processing AssignStmt Node: %s\n", node->assignStmt.varName);
        // Analyze the expression
        semanticAnalysis(node->assignStmt.expr, symTab);

        // Check that the variable is declared
        Symbol *assignSymbol = findSymbol(symTab, node->assignStmt.varName);
        if (assignSymbol == NULL)
        {
            fprintf(stderr, "Semantic error: Variable %s has not been declared, NodeType_AssignStmt\n", node->assignStmt.varName);
            exit(1);
        }

        // Check that the symbol is not an array
        if (assignSymbol->isArray)
        {
            fprintf(stderr, "Semantic error: Cannot assign to array name %s without index\n", node->assignStmt.varName);
            exit(1);
        }

        // Debugging statements
        printf("Variable '%s' type: %s\n", node->assignStmt.varName, assignSymbol->type);
        printf("Expression type: %s\n", node->assignStmt.expr->dataType);

        // Check type compatibility
        if (strcmp(assignSymbol->type, node->assignStmt.expr->dataType) != 0)
        {
            fprintf(stderr, "Semantic error: Type mismatch in assignment to variable %s\n", node->assignStmt.varName);
            exit(1);
        }

        // Generate TAC for the assignment
        generateTACForExpr(node, symTab);

        break;
    }

    case NodeType_ArrayAssign:
    {
        printf("Processing ArrayAssign Node: %s\n", node->arrayAssign.arrayName);
        // Analyze the index expression
        semanticAnalysis(node->arrayAssign.index, symTab);

        // Check that index is integer
        if (strcmp(node->arrayAssign.index->dataType, "int") != 0)
        {
            fprintf(stderr, "Semantic error: Array index must be an integer\n");
            exit(1);
        }

        // Analyze the expression
        semanticAnalysis(node->arrayAssign.expr, symTab);

        // Check that the array is declared
        Symbol *arraySymbol = findSymbol(symTab, node->arrayAssign.arrayName);
        if (arraySymbol == NULL || !arraySymbol->isArray)
        {
            fprintf(stderr, "Semantic error: Array %s has not been declared\n", node->arrayAssign.arrayName);
            exit(1);
        }

        // Debugging statements
        printf("Array '%s' type: %s\n", node->arrayAssign.arrayName, arraySymbol->type);
        printf("Expression type: %s\n", node->arrayAssign.expr->dataType);

        // Check type compatibility
        if (strcmp(arraySymbol->type, node->arrayAssign.expr->dataType) != 0)
        {
            fprintf(stderr, "Semantic error: Type mismatch in assignment to array %s\n", node->arrayAssign.arrayName);
            exit(1);
        }

        // Generate TAC for the array assignment
        generateTACForExpr(node, symTab);
        break;
    }

    case NodeType_BinOp:
        printf("Processing BinOp Node\n");
        semanticAnalysis(node->binOp.left, symTab);
        semanticAnalysis(node->binOp.right, symTab);

        // Rounding and Type Promotion Logic
        if (strcmp(node->binOp.left->dataType, "int") == 0 && strcmp(node->binOp.right->dataType, "float") == 0)
        {
            fprintf(stderr, "Warning: Implicit conversion of 'int' to 'float' with rounding on the left operand.\n");
            // Round the int value if necessary, promote result to float
            node->dataType = strdup("float");
        }
        else if (strcmp(node->binOp.left->dataType, "float") == 0 && strcmp(node->binOp.right->dataType, "int") == 0)
        {
            fprintf(stderr, "Warning: Implicit conversion of 'int' to 'float' with rounding on the right operand.\n");
            // Round the int value if necessary, promote result to float
            node->dataType = strdup("float");
        }
        else if (strcmp(node->binOp.left->dataType, "int") == 0 && strcmp(node->binOp.right->dataType, "int") == 0)
        {
            node->dataType = strdup("int"); // No promotion needed if both are int
        }
        else if (strcmp(node->binOp.left->dataType, "float") == 0 && strcmp(node->binOp.right->dataType, "float") == 0)
        {
            node->dataType = strdup("float"); // No promotion needed if both are float
        }
        else
        {
            fprintf(stderr, "Semantic warning: Type mismatch handled with implicit promotion in binary operation\n");
        }
        break;

    case NodeType_SimpleID:
    {
        printf("Processing SimpleID Node: %s\n", node->simpleID.name);
        Symbol *symbol = findSymbol(symTab, node->simpleID.name);
        if (symbol == NULL)
        {
            printAllSymbolTables(symTab);
            // printSymbolTable(symTab);
            fprintf(stderr, "Semantic error: Variable %s has not been declared, NodeType_SimpleID\n", node->simpleID.name);
            exit(1);
        }

        node->dataType = strdup(symbol->type);
        break;
    }

    case NodeType_SimpleExpr:
        printf("Processing SimpleExpr Node\n");
        node->dataType = node->simpleExpr.isFloat ? strdup("float") : strdup("int");
        break;

    case NodeType_WriteStmt:
        printf("Processing WriteStmt Node\n");
        semanticAnalysis(node->writeStmt.expr, symTab);

        // Generate TAC for the write statement
        generateTACForExpr(node, symTab);
        break;

    case NodeType_Block:
        printf("Processing Block Node\n");
        if (node->block.stmtList)
            semanticAnalysis(node->block.stmtList, symTab);
        break;

    case NodeType_ReturnStmt:
    {
        printf("Processing ReturnStmt Node\n");
        if (node->returnStmt.expr)
        {
            semanticAnalysis(node->returnStmt.expr, symTab);
        }
        break;
    }

    case NodeType_IfStmt:
        printf("Processing IfStmt Node\n");
        // Analyze the condition
        semanticAnalysis(node->ifStmt.condition, symTab);

        // Check that condition is boolean
        if (strcmp(node->ifStmt.condition->dataType, "bool") != 0 && strcmp(node->ifStmt.condition->dataType, "int") != 0)
        {
            fprintf(stderr, "Semantic error: Condition in if statement must be boolean or integer\n");
            exit(1);
        }

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
        printf("Processing WhileStmt Node\n");
        // Analyze the condition
        semanticAnalysis(node->whileStmt.condition, symTab);

        // Check that condition is boolean
        if (strcmp(node->whileStmt.condition->dataType, "bool") != 0 && strcmp(node->whileStmt.condition->dataType, "int") != 0)
        {
            fprintf(stderr, "Semantic error: Condition in while statement must be boolean or integer\n");
            exit(1);
        }

        // Analyze the block
        if (node->whileStmt.block)
            semanticAnalysis(node->whileStmt.block, symTab);

        // Generate TAC for the while loop
        generateTACForExpr(node, symTab);
        break;

    case NodeType_FuncCall:
    {
        printf("Processing FuncCall Node: %s\n", node->funcCall.funcName);
        // Analyze arguments
        ASTNode *argNode = node->funcCall.argList;
        int argCount = 0;
        while (argNode)
        {
            semanticAnalysis(argNode->argList.arg, symTab);
            argNode = argNode->argList.argList;
            argCount++;
        }

        // Check that the function is declared
        Symbol *funcSymbol = findSymbol(symTab, node->funcCall.funcName);
        if (funcSymbol == NULL || !funcSymbol->isFunction)
        {
            fprintf(stderr, "Semantic error: Function %s is not declared\n", node->funcCall.funcName);
            exit(1);
        }

        // Check number of arguments
        int paramCount = 0;
        ASTNode *paramNode = funcSymbol->paramList;
        while (paramNode)
        {
            paramNode = paramNode->paramList.nextParam;
            paramCount++;
        }

        if (argCount != paramCount)
        {
            fprintf(stderr, "Semantic error: Function %s expects %d arguments but %d were given\n",
                    node->funcCall.funcName, paramCount, argCount);
            exit(1);
        }

        // Additional checks for argument types can be added here

        // Generate TAC for the function call
        generateTACForExpr(node, symTab);
        break;
    }

    case NodeType_ArgList:
        printf("Processing ArgList Node\n");
        semanticAnalysis(node->argList.arg, symTab);
        semanticAnalysis(node->argList.argList, symTab);
        break;

    case NodeType_Arg:
        printf("Processing Arg Node\n");
        semanticAnalysis(node->arg.expr, symTab);
        node->dataType = strdup(node->arg.expr->dataType);
        break;

    case NodeType_LogicalOp:
        printf("Processing LogicalOp Node\n");
        semanticAnalysis(node->logicalOp.left, symTab);
        semanticAnalysis(node->logicalOp.right, symTab);

        // Check that operands are boolean or integer (since C allows non-zero integers as true)
        if ((strcmp(node->logicalOp.left->dataType, "bool") != 0 && strcmp(node->logicalOp.left->dataType, "int") != 0) ||
            (strcmp(node->logicalOp.right->dataType, "bool") != 0 && strcmp(node->logicalOp.right->dataType, "int") != 0))
        {
            fprintf(stderr, "Semantic error: Logical operators require boolean or integer operands\n");
            exit(1);
        }

        node->dataType = strdup("bool");
        break;

    case NodeType_RelOp:
        printf("Processing RelOp Node\n");
        semanticAnalysis(node->relOp.left, symTab);
        semanticAnalysis(node->relOp.right, symTab);

        // Type checking
        if (strcmp(node->relOp.left->dataType, node->relOp.right->dataType) != 0)
        {
            fprintf(stderr, "Semantic error: Type mismatch in relational operation\n");
            exit(1);
        }

        node->dataType = strdup("bool");
        break;

    case NodeType_NotOp:
        printf("Processing NotOp Node\n");
        semanticAnalysis(node->notOp.expr, symTab);

        // Check that operand is boolean or integer
        if (strcmp(node->notOp.expr->dataType, "bool") != 0 && strcmp(node->notOp.expr->dataType, "int") != 0)
        {
            fprintf(stderr, "Semantic error: NOT operator requires a boolean or integer operand\n");
            exit(1);
        }

        node->dataType = strdup("bool");
        break;

    case NodeType_ArrayAccess:
    {
        printf("Processing ArrayAccess Node: %s\n", node->arrayAccess.arrayName);
        // Analyze the index expression
        semanticAnalysis(node->arrayAccess.index, symTab);

        // Check that index is integer
        if (strcmp(node->arrayAccess.index->dataType, "int") != 0)
        {
            fprintf(stderr, "Semantic error: Array index must be an integer\n");
            exit(1);
        }

        // Check that the array is declared
        Symbol *symbol = findSymbol(symTab, node->arrayAccess.arrayName);
        if (symbol == NULL || !symbol->isArray)
        {
            fprintf(stderr, "Semantic error: Array %s has not been declared\n", node->arrayAccess.arrayName);
            exit(1);
        }

        // Set the data type based on the array's element type
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

        char *result = createTempVar(symTab, expr->dataType); // Use expr->dataType

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
            char *result = createTempVar(symTab, expr->dataType);
            ;

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
        char *result = createTempVar(symTab, expr->dataType);
        ;

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
        char *result = createTempVar(symTab, expr->dataType);

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
        char *result = createTempVar(symTab, expr->dataType);
        ;

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

char *createTempVar(SymbolTable *symTab, const char *dataType)
{
    static int tempVarCounter = 0;
    char *tempVarName = (char *)malloc(16);
    sprintf(tempVarName, "t%d", tempVarCounter++);

    // Insert temp variable into symbol table
    insertSymbol(symTab, tempVarName, dataType, false, false, NULL);

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
