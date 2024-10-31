#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
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
        // Analyze TAC for the program
        semanticAnalysis(node->program.funcDeclList, symTab);
        break;

    case NodeType_FuncDeclList:
        // Recursively analyze each function declaration in the list
        semanticAnalysis(node->funcDeclList.funcDecl, symTab);
        semanticAnalysis(node->funcDeclList.funcDeclList, symTab);
        break;

    case NodeType_FuncDecl:
    {
        // Check if the function name is already declared in the current scope
        if (findSymbol(symTab, node->funcDecl.funcName) != NULL)
        {
            fprintf(stderr, "Semantic error: Function %s is already declared\n", node->funcDecl.funcName);
            exit(1);
        }

        // Insert the function into the symbol table in the global scope
        insertSymbol(symTab, node->funcDecl.funcName, node->funcDecl.returnType, false, true, NULL);

        // Retrieve the symbol we just inserted
        Symbol *functionSymbol = findSymbol(symTab, node->funcDecl.funcName);

        // Set the parameter list of the function symbol
        setSymbolParamList(functionSymbol, node->funcDecl.paramList);

        // Create a new symbol table for the function's local scope, linked to the parent symTab
        SymbolTable *functionScope = createSymbolTable(TABLE_SIZE, symTab);

        // Assign a unique scope ID to functionScope
        functionScope->scope = GlobalScope++;

        // Add function parameters to the new scope
        ASTNode *paramNode = node->funcDecl.paramList;
        while (paramNode != NULL)
        {
            ASTNode *param = paramNode->paramList.param; // Get the parameter node
            if (param == NULL)
                break;

            if (findSymbol(functionScope, param->param.paramName) != NULL)
            {
                fprintf(stderr, "Semantic error: Parameter %s is already declared in function %s\n",
                        param->param.paramName, node->funcDecl.funcName);
                exit(1);
            }

            // Insert parameter into the function's symbol table
            insertSymbol(functionScope, param->param.paramName, param->param.paramType, false, false, NULL);

            paramNode = paramNode->paramList.nextParam;
        }

        // Perform semantic analysis on the function body within the new scope
        // semanticAnalysis(node->funcDecl.varDeclList, functionScope);
        // semanticAnalysis(node->funcDecl.block, functionScope);
        // semanticAnalysis(node->funcDecl.returnStmt, functionScope);

        // Generate TAC for the function
        generateTACForExpr(node, functionScope);
        break;
    }
    case NodeType_FuncCall:
    {
        // Check if the function is declared
        Symbol *functionSymbol = findSymbol(symTab, node->funcCall.funcName);
        if (functionSymbol == NULL)
        {
            fprintf(stderr, "Semantic error: Function %s is not declared\n", node->funcCall.funcName);
            exit(1);
        }

        // Retrieve the function's parameter list
        ASTNode *paramNode = functionSymbol->paramList;
        ASTNode *argNode = node->funcCall.argList;

        // Count parameters and arguments
        int paramCount = 0, argCount = 0;
        ASTNode *tmpNode = paramNode;
        while (tmpNode != NULL)
        {
            paramCount++;
            tmpNode = tmpNode->paramList.nextParam;
        }
        tmpNode = argNode;
        while (tmpNode != NULL)
        {
            argCount++;
            tmpNode = tmpNode->argList.argList;
        }
        if (paramCount != argCount)
        {
            fprintf(stderr, "Semantic error: Function %s expects %d arguments but %d were provided\n",
                    node->funcCall.funcName, paramCount, argCount);
            exit(1);
        }

        // Traverse parameters and arguments in parallel
        ASTNode *currParamNode = paramNode;
        ASTNode *currArgNode = argNode;
        while (currParamNode != NULL && currArgNode != NULL)
        {
            ASTNode *currParam = currParamNode->paramList.param; // NodeType_Param
            ASTNode *currArg = currArgNode->argList.arg;         // NodeType_Arg

            // Perform semantic analysis on the argument expression
            semanticAnalysis(currArg, symTab);

            // Check that the argument type matches the parameter type
            char *argType = currArg->dataType;
            char *paramType = currParam->param.paramType;

            if (strcmp(argType, paramType) != 0)
            {
                fprintf(stderr, "Semantic error: Argument type %s does not match parameter type %s in function %s\n",
                        argType, paramType, node->funcCall.funcName);
                exit(1);
            }

            currParamNode = currParamNode->paramList.nextParam;
            currArgNode = currArgNode->argList.argList;
        }

        // Generate TAC for the function call
        generateTACForExpr(node, symTab);
        break;
    }

    case NodeType_Arg:
    {
        if (node->arg.id != NULL)
        {
            // It's an identifier
            Symbol *symbol = findSymbol(symTab, node->arg.id);
            if (symbol == NULL)
            {
                fprintf(stderr, "Semantic error: Variable %s not declared\n", node->arg.id);
                exit(1);
            }
            node->dataType = strdup(symbol->type);
        }
        else if (node->arg.value != NULL)
        {
            // It's a literal value
            node->dataType = strdup(node->arg.type); // Assuming 'type' is set for literals
        }
        else
        {
            fprintf(stderr, "Semantic error: Invalid argument\n");
            exit(1);
        }
        break;
    }

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
                insertSymbol(symTab, node->varDecl.varName, node->varDecl.varType, false, false, NULL); // , NULL
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

        if ((strcmp(node->binOp.left->dataType, "int") == 0 && strcmp(node->binOp.right->dataType, "float") == 0) ||
            (strcmp(node->binOp.left->dataType, "float") == 0 && strcmp(node->binOp.right->dataType, "int") == 0))
        {
            node->dataType = strdup("float"); // Promote to float if mixed types
        }
        else if (strcmp(node->binOp.left->dataType, "float") == 0 && strcmp(node->binOp.right->dataType, "float") == 0)
        {
            node->dataType = strdup("float");
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

    case NodeType_ReturnStmt:
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
            insertSymbol(symTab, node->arrayDecl.varName, node->arrayDecl.varType, true, false, arrayInfo); // , 0, true
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
    case NodeType_Program:
    {
        // Process each function in the program independently
        if (expr->program.funcDeclList != NULL)
        {
            generateTACForExpr(expr->program.funcDeclList, symTab);
        }
    }
    break;

    case NodeType_FuncDeclList:
    {
        generateTACForExpr(expr->funcDeclList.funcDecl, symTab);
        if (expr->funcDeclList.funcDeclList)
        {
            generateTACForExpr(expr->funcDeclList.funcDeclList, symTab);
        }
    }
    break;

    case NodeType_FuncDecl:
        generateTACForFunction(expr, symTab);
        break;

    case NodeType_FuncCall:
    {
        // Collect arguments into a list
        ASTNode **args = NULL;
        int argCount = 0;
        int argCapacity = 0;

        ASTNode *argNode = expr->funcCall.argList;
        while (argNode != NULL)
        {
            // Expand array if needed
            if (argCount >= argCapacity)
            {
                argCapacity = argCapacity == 0 ? 4 : argCapacity * 2;
                args = realloc(args, argCapacity * sizeof(ASTNode *));
            }
            args[argCount++] = argNode->argList.arg;
            argNode = argNode->argList.argList;
        }

        // Process arguments from left to right
        for (int i = 0; i < argCount; i++)
        {
            char *argResult = generateTACForExpr(args[i], symTab);

            // Generate TAC instruction to pass argument
            TAC *paramTAC = (TAC *)malloc(sizeof(TAC));
            paramTAC->op = strdup("param");
            paramTAC->arg1 = strdup(argResult);
            paramTAC->arg2 = NULL;
            paramTAC->result = NULL;
            paramTAC->next = NULL;
            appendTAC(&tacHead, paramTAC);
        }

        free(args);

        // Generate TAC for the function call
        TAC *callTAC = (TAC *)malloc(sizeof(TAC));
        callTAC->op = strdup("call");
        callTAC->arg1 = strdup(expr->funcCall.funcName);
        callTAC->arg2 = NULL;
        callTAC->result = NULL;
        callTAC->next = NULL;
        appendTAC(&tacHead, callTAC);

        break;
    }

    case NodeType_Arg:
    {
        if (expr->arg.id != NULL)
        {
            // It's an identifier
            return strdup(expr->arg.id);
        }
        else if (expr->arg.value != NULL)
        {
            // It's a literal value
            return strdup(expr->arg.value);
        }
        else
        {
            fprintf(stderr, "Error: Invalid argument\n");
            return NULL;
        }
    }

    case NodeType_AssignStmt:
    {
        // Generate TAC for the right-hand side expression
        char *rhs = generateTACForExpr(expr->assignStmt.expr, symTab);

        printf("%s\n", rhs);

        // Find the type of the left-hand side variable in the symbol table
        Symbol *symbol = findSymbol(symTab, expr->assignStmt.varName);

        if (symbol && strcmp(symbol->type, "float") == 0)
        {
            // Update the value of the float variable in the symbol table
            // float rhsValue = atof(rhs); // Ensure rhs is interpreted as a float
            updateSymbolValue(symTab, expr->assignStmt.varName, rhs);

            // Create a TAC instruction for the assignment
            TAC *assignTAC = (TAC *)malloc(sizeof(TAC));
            assignTAC->op = strdup("fmov"); // Use fmov for floating-point assignment
            assignTAC->arg1 = strdup(rhs);
            assignTAC->arg2 = NULL;
            assignTAC->result = strdup(expr->assignStmt.varName);
            assignTAC->next = NULL;

            appendTAC(&tacHead, assignTAC);
        }
        else
        {
            // Handle integer or other types of assignment
            updateSymbolValue(symTab, expr->assignStmt.varName, rhs);

            printf("Angel\n");

            // Create a TAC instruction for the assignment
            TAC *assignTAC = (TAC *)malloc(sizeof(TAC));
            assignTAC->op = strdup("=");
            assignTAC->arg1 = strdup(rhs);
            assignTAC->arg2 = NULL;
            assignTAC->result = strdup(expr->assignStmt.varName);
            assignTAC->next = NULL;

            appendTAC(&tacHead, assignTAC);
        }

        return strdup(expr->assignStmt.varName);
    }
    break;

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
        const char *resultReg = isFloatOp ? allocateFloatRegister() : allocateRegister();
        if (!resultReg)
        {
            fprintf(stderr, "Error: No available registers for result\n");
            exit(1);
        }

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
        binOpTAC->result = strdup(resultReg); // Store result in the allocated register
        binOpTAC->next = NULL;

        appendTAC(&tacHead, binOpTAC);

        return strdup(resultReg);
    }
    break;

    case NodeType_SimpleExpr:
    {
        char buffer[20]; // Buffer to hold the numeric literal

        // Ensure the data type is correctly recognized
        if (expr->simpleExpr.isFloat) // Assuming isFloat is set for floats
        {
            expr->dataType = strdup("float");
            snprintf(buffer, sizeof(buffer), "%.6f", expr->simpleExpr.floatValue);
        }
        else
        {
            expr->dataType = strdup("int");
            snprintf(buffer, sizeof(buffer), "%d", expr->simpleExpr.number);
        }

        // Return the numeric constant as a string
        return strdup(buffer);
    }
    break;

    case NodeType_SimpleID:
    {
        Symbol *symbol = findSymbol(symTab, expr->simpleID.name);
        if (symbol && strcmp(symbol->type, "float") == 0)
        {
            // Return the variable name and mark it as a float
            return strdup(expr->simpleID.name); // Floating-point variable
        }
        else
        {
            // Return the variable name (default integer behavior)
            return strdup(expr->simpleID.name);
        }
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
            // Handle constant write for integers
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

            // Determine if the expression result is a float or an integer
            if (strcmp(foundSymbol->type, "float") == 0)
            {
                // Handle write for floating-point values
                writeTAC->op = strdup("write_float");
            }
            else
            {
                // Handle write for integer values (default case)
                writeTAC->op = strdup("write");
            }

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
        arrayAccessTAC->result = createTempVar(symTab);             // Create a temporary variable to hold the result
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

void generateTACForFunction(ASTNode *funcNode, SymbolTable *symTab)
{
    if (!funcNode || !funcNode->funcDecl.funcName)
        return;

    // Generate function label
    TAC *labelTAC = (TAC *)malloc(sizeof(TAC));
    labelTAC->op = strdup("label");
    labelTAC->arg1 = strdup(funcNode->funcDecl.funcName);
    labelTAC->arg2 = NULL;
    labelTAC->result = NULL;
    appendTAC(&tacHead, labelTAC);

    // Generate prologue
    TAC *prologueTAC = (TAC *)malloc(sizeof(TAC));
    prologueTAC->op = strdup("prologue");
    prologueTAC->arg1 = strdup(funcNode->funcDecl.funcName);
    prologueTAC->arg2 = NULL;
    prologueTAC->result = NULL;
    appendTAC(&tacHead, prologueTAC);

    // Analyze and generate TAC for function body within the new scope
    if (funcNode->funcDecl.varDeclList)
    {
        semanticAnalysis(funcNode->funcDecl.varDeclList, symTab);
    }
    if (funcNode->funcDecl.block)
    {
        semanticAnalysis(funcNode->funcDecl.block, symTab);
    }
    if (funcNode->funcDecl.returnStmt)
    {
        semanticAnalysis(funcNode->funcDecl.returnStmt, symTab);
    }

    // Generate epilogue
    TAC *epilogueTAC = (TAC *)malloc(sizeof(TAC));
    epilogueTAC->op = strdup("epilogue");
    epilogueTAC->arg1 = strdup(funcNode->funcDecl.funcName);
    epilogueTAC->arg2 = NULL;
    epilogueTAC->result = NULL;
    appendTAC(&tacHead, epilogueTAC);

    // Generate return statement
    TAC *returnTAC = (TAC *)malloc(sizeof(TAC));
    returnTAC->op = strdup("return");
    returnTAC->arg1 = NULL;
    returnTAC->arg2 = NULL;
    returnTAC->result = NULL;
    appendTAC(&tacHead, returnTAC);
}

// Function to create a new temporary variable for TAC
char *createTempVar(SymbolTable *symTab)
{
    int index = allocateNextAvailableTempVar(tempVars, 50);
    if (index == -1)
    {
        fprintf(stderr, "Error: No available temporary variables\n");
        return NULL;
    }

    char *tempVar = malloc(10); // Enough space for "t" + number
    if (!tempVar)
        return NULL;

    sprintf(tempVar, "t%d", index);

    // Insert the temporary variable into the symbol table
    if (findSymbol(symTab, tempVar) == NULL)
    {
        insertSymbol(symTab, tempVar, "int", false, false, NULL); // Assuming temporaries are of type int , NULL
    }

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
