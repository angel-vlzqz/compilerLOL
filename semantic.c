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
        semanticAnalysis(node->program.declList, symTab);
        break;

    case NodeType_DeclList:
        printf("Processing DeclList Node at address %p\n", (void*)node);
        semanticAnalysis(node->declList.decl, symTab);
        if (node->declList.next)
        {
            printf("Moving to next DeclList Node at address %p\n", (void*)node->declList.next);
            semanticAnalysis(node->declList.next, symTab);
        }
        else
        {
            printf("No more DeclList Nodes\n");
        }
        break;

    case NodeType_FuncDecl:
    {
        printf("Processing FuncDecl Node: %s\n", node->funcDecl.funcName);

        SymbolTable *functionScope = node->funcDecl.symTab;
        if (functionScope == NULL)
        {
            functionScope = createSymbolTable(TABLE_SIZE, symTab);
            node->funcDecl.symTab = functionScope;
        }

        Symbol *funcSymbol = findSymbolInCurrentScope(symTab, node->funcDecl.funcName);
        if (funcSymbol == NULL)
        {
            insertSymbol(symTab, node->funcDecl.funcName, node->funcDecl.returnType, false, true, NULL);
            funcSymbol = findSymbolInCurrentScope(symTab, node->funcDecl.funcName);
            if (funcSymbol == NULL)
            {
                fprintf(stderr, "Error: Failed to insert function %s into symbol table\n", node->funcDecl.funcName);
                exit(1);
            }
            setSymbolParamList(funcSymbol, node->funcDecl.paramList);
        }
        else
        {
            fprintf(stderr, "Semantic error: Function %s is already declared\n", node->funcDecl.funcName);
            exit(1);
        }

        // Add parameters to the function's symbol table
        ASTNode *paramListNode = node->funcDecl.paramList;
        while (paramListNode)
        {
            ASTNode *paramNode = paramListNode->paramList.param;
            if (paramNode->type == NodeType_Param)
            {
                Symbol *existingParam = findSymbolInCurrentScope(functionScope, paramNode->param.paramName);
                if (existingParam != NULL)
                {
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

        semanticAnalysis(node->funcDecl.varDeclList, functionScope);
        semanticAnalysis(node->funcDecl.block, functionScope);
        semanticAnalysis(node->funcDecl.returnStmt, functionScope);

        // Generate TAC for the function
        generateTACForFunction(node, functionScope);

        break;
    }

    case NodeType_VarDeclList:
        printf("Processing VarDeclList Node\n");
        semanticAnalysis(node->varDeclList.varDecl, symTab);
        semanticAnalysis(node->varDeclList.varDeclList, symTab);
        break;

    case NodeType_ParamList:
        printf("Processing ParamList Node\n");
        semanticAnalysis(node->paramList.param, symTab);
        semanticAnalysis(node->paramList.nextParam, symTab);
        break;

    case NodeType_Param:
    {
        printf("Processing Param Node: %s\n", node->param.paramName);

        Symbol *existingSymbol = findSymbolInCurrentScope(symTab, node->param.paramName);
        if (existingSymbol != NULL)
        {
            fprintf(stderr, "Semantic error: Parameter %s is already declared in this scope\n", node->param.paramName);
            exit(1);
        }

        insertSymbol(symTab, node->param.paramName, node->param.paramType, false, false, NULL);

        break;
    }

    case NodeType_VarDecl:
    {
        printf("Processing VarDecl Node: %s\n", node->varDecl.varName);

        Symbol *existingSymbol = findSymbolInCurrentScope(symTab, node->varDecl.varName);
        if (existingSymbol != NULL)
        {
            fprintf(stderr, "Semantic error: Variable %s is already declared in this scope\n", node->varDecl.varName);
            exit(1);
        }

        // Add this section to handle initial values
        if (node->varDecl.initialValue != NULL)
        {
            char valueStr[32];
            if (strcmp(node->varDecl.varType, "float") == 0)
            {
                snprintf(valueStr, sizeof(valueStr), "%f", node->varDecl.initialValue->simpleExpr.floatValue);
            }
            else
            {
                snprintf(valueStr, sizeof(valueStr), "%d", node->varDecl.initialValue->simpleExpr.number);
            }
            insertSymbol(symTab, node->varDecl.varName, node->varDecl.varType, false, false, NULL);
            updateSymbolValue(symTab, node->varDecl.varName, valueStr);
        }
        else
        {
            insertSymbol(symTab, node->varDecl.varName, node->varDecl.varType, false, false, NULL);
        }
        break;
    }

    case NodeType_ArrayDecl:
    {
        printf("Processing ArrayDecl Node: %s\n", node->arrayDecl.varName);

        Symbol *existingSymbol = findSymbolInCurrentScope(symTab, node->arrayDecl.varName);
        if (existingSymbol != NULL)
        {
            fprintf(stderr, "Semantic error: Array %s is already declared in this scope\n", node->arrayDecl.varName);
            exit(1);
        }

        Array *arrayInfo = createArray(node->arrayDecl.varType, node->arrayDecl.size);
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
        semanticAnalysis(node->assignStmt.expr, symTab);

        Symbol *assignSymbol = findSymbol(symTab, node->assignStmt.varName);
        if (assignSymbol == NULL)
        {
            fprintf(stderr, "Semantic error: Variable %s has not been declared\n", node->assignStmt.varName);
            exit(1);
        }

        if (assignSymbol->isArray)
        {
            fprintf(stderr, "Semantic error: Cannot assign to array name %s without index\n", node->assignStmt.varName);
            exit(1);
        }

        // Check types and allow automatic casting
        if (strcmp(assignSymbol->type, node->assignStmt.expr->dataType) != 0)
        {
            // Attempt automatic casting if one is int and the other is float
            if ((strcmp(assignSymbol->type, "int") == 0 && strcmp(node->assignStmt.expr->dataType, "float") == 0) ||
                (strcmp(assignSymbol->type, "float") == 0 && strcmp(node->assignStmt.expr->dataType, "int") == 0))
            {
                fprintf(stderr, "Info: Automatically casting %s to %s in assignment to %s.\n",
                        node->assignStmt.expr->dataType, assignSymbol->type, node->assignStmt.varName);
                // Set the expression's dataType to the variable's type after casting
                free(node->assignStmt.expr->dataType);
                node->assignStmt.expr->dataType = strdup(assignSymbol->type);
            }
            else
            {
                fprintf(stderr, "Semantic error: Type mismatch in assignment to variable %s (cannot cast)\n", node->assignStmt.varName);
                exit(1);
            }
        }

        // Update symbol value if it's a simple expression
        if (node->assignStmt.expr->type == NodeType_SimpleExpr)
        {
            char valueStr[32];
            if (node->assignStmt.expr->simpleExpr.isFloat)
            {
                snprintf(valueStr, sizeof(valueStr), "%f", node->assignStmt.expr->simpleExpr.floatValue);
            }
            else
            {
                snprintf(valueStr, sizeof(valueStr), "%d", node->assignStmt.expr->simpleExpr.number);
            }
            updateSymbolValue(symTab, node->assignStmt.varName, valueStr);
        }

        generateTACForExpr(node, symTab);

        break;
    }

    case NodeType_ArrayAssign:
    {
        printf("Processing ArrayAssign Node: %s\n", node->arrayAssign.arrayName);
        semanticAnalysis(node->arrayAssign.index, symTab);

        if (strcmp(node->arrayAssign.index->dataType, "int") != 0)
        {
            fprintf(stderr, "Semantic error: Array index must be an integer\n");
            exit(1);
        }

        semanticAnalysis(node->arrayAssign.expr, symTab);

        Symbol *arraySymbol = findSymbol(symTab, node->arrayAssign.arrayName);
        if (arraySymbol == NULL || !arraySymbol->isArray)
        {
            fprintf(stderr, "Semantic error: Array %s has not been declared\n", node->arrayAssign.arrayName);
            exit(1);
        }

        // Attempt automatic casting for array assignments
        if (strcmp(arraySymbol->type, node->arrayAssign.expr->dataType) != 0)
        {
            if ((strcmp(arraySymbol->type, "int") == 0 && strcmp(node->arrayAssign.expr->dataType, "float") == 0) ||
                (strcmp(arraySymbol->type, "float") == 0 && strcmp(node->arrayAssign.expr->dataType, "int") == 0))
            {
                fprintf(stderr, "Info: Automatically casting %s to %s in assignment to array %s.\n",
                        node->arrayAssign.expr->dataType, arraySymbol->type, node->arrayAssign.arrayName);
                free(node->arrayAssign.expr->dataType);
                node->arrayAssign.expr->dataType = strdup(arraySymbol->type);
            }
            else
            {
                fprintf(stderr, "Semantic error: Type mismatch in assignment to array %s (cannot cast)\n", node->arrayAssign.arrayName);
                exit(1);
            }
        }

        generateTACForExpr(node, symTab);
        break;
    }

    case NodeType_BinOp:
        printf("Processing BinOp Node\n");
        semanticAnalysis(node->binOp.left, symTab);
        semanticAnalysis(node->binOp.right, symTab);

        // Handle automatic casting for binary operations
        if (strcmp(node->binOp.left->dataType, "int") == 0 && strcmp(node->binOp.right->dataType, "float") == 0)
        {
            fprintf(stderr, "Info: Automatically casting int to float in binary operation.\n");
            node->dataType = strdup("float");
        }
        else if (strcmp(node->binOp.left->dataType, "float") == 0 && strcmp(node->binOp.right->dataType, "int") == 0)
        {
            fprintf(stderr, "Info: Automatically casting int to float in binary operation.\n");
            node->dataType = strdup("float");
        }
        else if (strcmp(node->binOp.left->dataType, node->binOp.right->dataType) == 0)
        {
            // Same type, just assign
            node->dataType = strdup(node->binOp.left->dataType);
        }
        else
        {
            fprintf(stderr, "Semantic warning: Complex type mismatch, attempted implicit promotion failed.\n");
            // Handle other complex mismatches if needed
            // For now, let's just default to float
            node->dataType = strdup("float");
        }
        break;

    case NodeType_SimpleID:
    {
        printf("Processing SimpleID Node: %s\n", node->simpleID.name);
        Symbol *symbol = findSymbol(symTab, node->simpleID.name);
        if (symbol == NULL)
        {
            printAllSymbolTables(symTab);
            fprintf(stderr, "Semantic error: Variable %s has not been declared\n", node->simpleID.name);
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
        semanticAnalysis(node->ifStmt.condition, symTab);

        if (strcmp(node->ifStmt.condition->dataType, "bool") != 0 && strcmp(node->ifStmt.condition->dataType, "int") != 0)
        {
            fprintf(stderr, "Semantic error: Condition in if statement must be boolean or integer\n");
            exit(1);
        }

        if (node->ifStmt.thenBlock)
            semanticAnalysis(node->ifStmt.thenBlock, symTab);

        if (node->ifStmt.elseBlock)
            semanticAnalysis(node->ifStmt.elseBlock, symTab);

        generateTACForExpr(node, symTab);
        break;

    case NodeType_WhileStmt:
        printf("Processing WhileStmt Node\n");
        semanticAnalysis(node->whileStmt.condition, symTab);

        if (strcmp(node->whileStmt.condition->dataType, "bool") != 0 && strcmp(node->whileStmt.condition->dataType, "int") != 0)
        {
            fprintf(stderr, "Semantic error: Condition in while statement must be boolean or integer\n");
            exit(1);
        }

        if (node->whileStmt.block)
            semanticAnalysis(node->whileStmt.block, symTab);

        generateTACForExpr(node, symTab);
        break;

    case NodeType_FuncCall:
    {
        printf("Processing FuncCall Node: %s\n", node->funcCall.funcName);
        ASTNode *argNode = node->funcCall.argList;
        int argCount = 0;
        while (argNode)
        {
            semanticAnalysis(argNode->argList.arg, symTab);
            argNode = argNode->argList.argList;
            argCount++;
        }

        Symbol *funcSymbol = findSymbol(symTab, node->funcCall.funcName);
        if (funcSymbol == NULL || !funcSymbol->isFunction)
        {
            fprintf(stderr, "Semantic error: Function %s is not declared\n", node->funcCall.funcName);
            exit(1);
        }

        node->dataType = strdup(funcSymbol->type);

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
        semanticAnalysis(node->arrayAccess.index, symTab);

        if (strcmp(node->arrayAccess.index->dataType, "int") != 0)
        {
            fprintf(stderr, "Semantic error: Array index must be an integer\n");
            exit(1);
        }

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
        char *rhs = generateTACForExpr(expr->assignStmt.expr, symTab);
        char *lhs = expr->assignStmt.varName;

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
        // Generate TAC for left and right operands
        char *left = generateTACForExpr(expr->binOp.left, symTab);
        char *right = generateTACForExpr(expr->binOp.right, symTab);

        // Check the data types of the operands
        Symbol *leftSymbol = findSymbol(symTab, left);
        Symbol *rightSymbol = findSymbol(symTab, right);
        bool isFloatOp = false;

        if ((leftSymbol && strcmp(leftSymbol->type, "float") == 0) ||
            (rightSymbol && strcmp(rightSymbol->type, "float") == 0))
        {
            isFloatOp = true;
        }

        // Allocate a register for the result
        char *result = createTempVar(symTab, expr->dataType);
        char op[4];
        snprintf(op, sizeof(op), "%c", expr->binOp.operator);

        // Create a TAC instruction for the binary operation
        TAC *binOpTAC = (TAC *)malloc(sizeof(TAC));
        char opStr[5]; // Need more space to differentiate between fadd, fsub, etc.
        if (isFloatOp)
        {
            if (expr->binOp.operator== '+')
            {
                strcpy(opStr, "fadd");
            }
            else if (expr->binOp.operator== '-')
            {
                strcpy(opStr, "fsub");
            }
            else if (expr->binOp.operator== '*')
            {
                strcpy(opStr, "fmul");
            }
            else if (expr->binOp.operator== '/')
            {
                strcpy(opStr, "fdiv");
            }
        }
        else
        {
            opStr[0] = expr->binOp.operator;
            opStr[1] = '\0';
        }

        binOpTAC->op = strdup(opStr);
        binOpTAC->arg1 = strdup(left);
        binOpTAC->arg2 = strdup(right);
        binOpTAC->result = strdup(result); // Store result in the allocated register
        binOpTAC->next = NULL;

        appendTAC(&tacHead, binOpTAC);

        return strdup(result);
    }

    case NodeType_SimpleExpr:
    {
        char buffer[32];
        if (expr->simpleExpr.isFloat)
            snprintf(buffer, sizeof(buffer), "%.6f", expr->simpleExpr.floatValue);
        else
            snprintf(buffer, sizeof(buffer), "%d", expr->simpleExpr.number);

        return strdup(buffer);
    }

    case NodeType_SimpleID:
        return strdup(expr->simpleID.name);

    case NodeType_WriteStmt:
    {
        char *exprResult = generateTACForExpr(expr->writeStmt.expr, symTab);
        if (strcmp(expr->writeStmt.expr->dataType, "float") == 0)
        {
            TAC *writeTAC = (TAC *)malloc(sizeof(TAC));
            writeTAC->op = strdup("write_float");
            writeTAC->arg1 = strdup(exprResult);
            writeTAC->arg2 = NULL;
            writeTAC->result = NULL;
            writeTAC->next = NULL;
            appendTAC(&tacHead, writeTAC);
        }
        else
        {
            TAC *writeTAC = (TAC *)malloc(sizeof(TAC));
            writeTAC->op = strdup("write");
            writeTAC->arg1 = strdup(exprResult);
            writeTAC->arg2 = NULL;
            writeTAC->result = NULL;
            writeTAC->next = NULL;
            appendTAC(&tacHead, writeTAC);
        }
        return NULL;
    }

    case NodeType_IfStmt:
    {
        char *labelFalse = createLabel();
        char *labelEnd = createLabel();

        char *condResult = generateTACForExpr(expr->ifStmt.condition, symTab);

        TAC *ifTAC = (TAC *)malloc(sizeof(TAC));
        ifTAC->op = strdup("ifFalse");
        ifTAC->arg1 = strdup(condResult);
        ifTAC->arg2 = NULL;
        ifTAC->result = strdup(labelFalse);
        ifTAC->next = NULL;
        appendTAC(&tacHead, ifTAC);

        generateTACForExpr(expr->ifStmt.thenBlock, symTab);

        TAC *gotoEndTAC = (TAC *)malloc(sizeof(TAC));
        gotoEndTAC->op = strdup("goto");
        gotoEndTAC->arg1 = strdup(labelEnd);
        gotoEndTAC->arg2 = NULL;
        gotoEndTAC->result = NULL;
        gotoEndTAC->next = NULL;
        appendTAC(&tacHead, gotoEndTAC);

        TAC *labelFalseTAC = (TAC *)malloc(sizeof(TAC));
        labelFalseTAC->op = strdup("label");
        labelFalseTAC->arg1 = strdup(labelFalse);
        labelFalseTAC->arg2 = NULL;
        labelFalseTAC->result = NULL;
        labelFalseTAC->next = NULL;
        appendTAC(&tacHead, labelFalseTAC);

        if (expr->ifStmt.elseBlock)
            generateTACForExpr(expr->ifStmt.elseBlock, symTab);

        TAC *labelEndTAC = (TAC *)malloc(sizeof(TAC));
        labelEndTAC->op = strdup("label");
        labelEndTAC->arg1 = strdup(labelEnd);
        labelEndTAC->arg2 = NULL;
        labelEndTAC->result = NULL;
        labelEndTAC->next = NULL;
        appendTAC(&tacHead, labelEndTAC);

        free(labelFalse);
        free(labelEnd);

        return NULL;
    }

    case NodeType_WhileStmt:
    {
        char *labelStart = createLabel();
        char *labelEnd = createLabel();

        TAC *labelStartTAC = (TAC *)malloc(sizeof(TAC));
        labelStartTAC->op = strdup("label");
        labelStartTAC->arg1 = strdup(labelStart);
        labelStartTAC->arg2 = NULL;
        labelStartTAC->result = NULL;
        labelStartTAC->next = NULL;
        appendTAC(&tacHead, labelStartTAC);

        char *condResult = generateTACForExpr(expr->whileStmt.condition, symTab);

        TAC *ifFalseTAC = (TAC *)malloc(sizeof(TAC));
        ifFalseTAC->op = strdup("ifFalse");
        ifFalseTAC->arg1 = strdup(condResult);
        ifFalseTAC->arg2 = NULL;
        ifFalseTAC->result = strdup(labelEnd);
        ifFalseTAC->next = NULL;
        appendTAC(&tacHead, ifFalseTAC);

        generateTACForExpr(expr->whileStmt.block, symTab);

        TAC *gotoStartTAC = (TAC *)malloc(sizeof(TAC));
        gotoStartTAC->op = strdup("goto");
        gotoStartTAC->arg1 = strdup(labelStart);
        gotoStartTAC->arg2 = NULL;
        gotoStartTAC->result = NULL;
        gotoStartTAC->next = NULL;
        appendTAC(&tacHead, gotoStartTAC);

        TAC *labelEndTAC = (TAC *)malloc(sizeof(TAC));
        labelEndTAC->op = strdup("label");
        labelEndTAC->arg1 = strdup(labelEnd);
        labelEndTAC->arg2 = NULL;
        labelEndTAC->result = NULL;
        labelEndTAC->next = NULL;
        appendTAC(&tacHead, labelEndTAC);

        free(labelStart);
        free(labelEnd);

        return NULL;
    }

    case NodeType_FuncCall:
    {
        ASTNode *argNode = expr->funcCall.argList;
        while (argNode)
        {
            char *argResult = generateTACForExpr(argNode->argList.arg, symTab);
            TAC *paramTAC = (TAC *)malloc(sizeof(TAC));
            paramTAC->op = strdup("param");
            paramTAC->arg1 = strdup(argResult);
            paramTAC->arg2 = NULL;
            paramTAC->result = NULL;
            paramTAC->next = NULL;
            appendTAC(&tacHead, paramTAC);

            argNode = argNode->argList.argList;
        }

        TAC *callTAC = (TAC *)malloc(sizeof(TAC));
        callTAC->op = strdup("call");
        callTAC->arg1 = strdup(expr->funcCall.funcName);
        callTAC->arg2 = NULL;
        callTAC->result = NULL;
        callTAC->next = NULL;
        appendTAC(&tacHead, callTAC);

        Symbol *funcSymbol = findSymbol(symTab, expr->funcCall.funcName);
        if (funcSymbol && strcmp(funcSymbol->type, "void") != 0)
        {
            char *result = createTempVar(symTab, funcSymbol->type);

            TAC *retTAC = (TAC *)malloc(sizeof(TAC));
            retTAC->op = strdup("=");
            retTAC->arg1 = strdup("v0");
            retTAC->arg2 = NULL;
            retTAC->result = strdup(result);
            retTAC->next = NULL;
            appendTAC(&tacHead, retTAC);

            return result;
        }

        return NULL;
    }

    case NodeType_ArgList:
    {
        if (expr->argList.arg)
            generateTACForExpr(expr->argList.arg, symTab);
        if (expr->argList.argList)
            generateTACForExpr(expr->argList.argList, symTab);
        return NULL;
    }

    case NodeType_Arg:
    {
        return generateTACForExpr(expr->arg.expr, symTab);
    }

    case NodeType_LogicalOp:
    {
        char *left = generateTACForExpr(expr->logicalOp.left, symTab);
        char *right = generateTACForExpr(expr->logicalOp.right, symTab);
        char *result = createTempVar(symTab, expr->dataType);

        TAC *logicalTAC = (TAC *)malloc(sizeof(TAC));
        logicalTAC->op = strdup(expr->logicalOp.logicalOp);
        logicalTAC->arg1 = strdup(left);
        logicalTAC->arg2 = strdup(right);
        logicalTAC->result = strdup(result);
        logicalTAC->next = NULL;

        appendTAC(&tacHead, logicalTAC);

        return result;
    }

    case NodeType_RelOp:
    {
        char *left = generateTACForExpr(expr->relOp.left, symTab);
        char *right = generateTACForExpr(expr->relOp.right, symTab);
        char *result = createTempVar(symTab, expr->dataType);

        TAC *relOpTAC = (TAC *)malloc(sizeof(TAC));
        relOpTAC->op = strdup(expr->relOp.operator);
        relOpTAC->arg1 = strdup(left);
        relOpTAC->arg2 = strdup(right);
        relOpTAC->result = strdup(result);
        relOpTAC->next = NULL;

        appendTAC(&tacHead, relOpTAC);

        return result;
    }

    case NodeType_NotOp:
    {
        char *exprResult = generateTACForExpr(expr->notOp.expr, symTab);
        char *result = createTempVar(symTab, expr->dataType);

        TAC *notOpTAC = (TAC *)malloc(sizeof(TAC));
        notOpTAC->op = strdup("!");
        notOpTAC->arg1 = strdup(exprResult);
        notOpTAC->arg2 = NULL;
        notOpTAC->result = strdup(result);
        notOpTAC->next = NULL;

        appendTAC(&tacHead, notOpTAC);

        return result;
    }

    case NodeType_ArrayAccess:
    {
        printf("Generating TAC for array access\n");
        char *index = generateTACForExpr(expr->arrayAccess.index, symTab);
        char *result = createTempVar(symTab, expr->dataType);

        // Instead of array_load, use '=[]'
        TAC *arrayTAC = (TAC *)malloc(sizeof(TAC));
        arrayTAC->op = strdup("=[]");
        arrayTAC->arg1 = strdup(expr->arrayAccess.arrayName);
        arrayTAC->arg2 = strdup(index);
        arrayTAC->result = strdup(result);
        arrayTAC->next = NULL;

        appendTAC(&tacHead, arrayTAC);

        return result;
    }

    case NodeType_ArrayAssign:
    {
        // This was handled by semantic. We must generate []= here
        char *index = generateTACForExpr(expr->arrayAssign.index, symTab);
        char *rhs = generateTACForExpr(expr->arrayAssign.expr, symTab);

        TAC *assignTAC = (TAC *)malloc(sizeof(TAC));
        assignTAC->op = strdup("[]=");
        assignTAC->arg1 = strdup(expr->arrayAssign.arrayName);
        assignTAC->arg2 = strdup(index);
        assignTAC->result = strdup(rhs);
        assignTAC->next = NULL;

        appendTAC(&tacHead, assignTAC);
        printf("Generated TAC for array assign: %s[%s] = %s\n", expr->arrayAssign.arrayName, index, rhs);

        return NULL;
    }

    case NodeType_Block:
        if (expr->block.stmtList)
            generateTACForExpr(expr->block.stmtList, symTab);
        return NULL;

    case NodeType_StmtList:
    {
        if (expr->stmtList.stmt)
            generateTACForExpr(expr->stmtList.stmt, symTab);
        if (expr->stmtList.stmtList)
            generateTACForExpr(expr->stmtList.stmtList, symTab);
        return NULL;
    }

    case NodeType_ReturnStmt:
    {
        char *retValue = NULL;
        if (expr->returnStmt.expr)
            retValue = generateTACForExpr(expr->returnStmt.expr, symTab);

        TAC *returnTAC = (TAC *)malloc(sizeof(TAC));
        returnTAC->op = strdup("return");
        returnTAC->arg1 = retValue ? strdup(retValue) : NULL;
        returnTAC->arg2 = NULL;
        returnTAC->result = NULL;
        returnTAC->next = NULL;

        appendTAC(&tacHead, returnTAC);

        return NULL;
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

    TAC *labelTAC = (TAC *)malloc(sizeof(TAC));
    labelTAC->op = strdup("label");
    labelTAC->arg1 = strdup(funcNode->funcDecl.funcName);
    labelTAC->arg2 = NULL;
    labelTAC->result = NULL;
    labelTAC->next = NULL;
    appendTAC(&tacHead, labelTAC);

    ASTNode *paramList = funcNode->funcDecl.paramList;
    while (paramList != NULL)
    {
        TAC *paramTAC = (TAC *)malloc(sizeof(TAC));
        paramTAC->op = strdup("param");
        paramTAC->arg1 = strdup(paramList->paramList.param->param.paramName);
        paramTAC->arg2 = strdup(paramList->paramList.param->param.paramType);
        paramTAC->result = NULL;
        paramTAC->next = NULL;
        appendTAC(&tacHead, paramTAC);

        paramList = paramList->paramList.nextParam;
    }

    TAC *prologueTAC = (TAC *)malloc(sizeof(TAC));
    prologueTAC->op = strdup("prologue");
    prologueTAC->arg1 = strdup(funcNode->funcDecl.funcName);
    prologueTAC->arg2 = NULL;
    prologueTAC->result = NULL;
    prologueTAC->next = NULL;
    appendTAC(&tacHead, prologueTAC);

    if (funcNode->funcDecl.block)
        generateTACForExpr(funcNode->funcDecl.block, symTab);

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