#include <stdio.h>
#include <string.h>
#include "semantic.h"
#include "temp.h"

int tempVars[20] = {0}; // Definition and initialization

// Perform semantic analysis on the AST
TAC* tacHead = NULL;

void semanticAnalysis(ASTNode* node, SymbolTable* symTab) {
    if (node == NULL) return;

    switch (node->type) {
         case NodeType_Program:
            printf("Performing semantic analysis on program\n");
            semanticAnalysis(node->program.varDeclList, symTab);
            semanticAnalysis(node->program.stmtList, symTab);
            break;
        case NodeType_VarDeclList:
            semanticAnalysis(node->varDeclList.varDecl, symTab);
            semanticAnalysis(node->varDeclList.varDeclList, symTab);
            break;
        case NodeType_VarDecl:
            // Check for redeclaration of variables
            if (findSymbol(symTab, node->varDecl.varName) != NULL) {
                fprintf(stderr, "Semantic error: Variable %s is already declared\n", node->varDecl.varName);
            } else {
                insertSymbol(symTab, node->varDecl.varName, node->varDecl.varType);
            }
            break;
        case NodeType_StmtList:
            semanticAnalysis(node->stmtList.stmt, symTab);
            semanticAnalysis(node->stmtList.stmtList, symTab);
            break;
        case NodeType_AssignStmt:
            if (findSymbol(symTab, node->assignStmt.varName) == NULL) {
                fprintf(stderr, "Semantic error: Variable %s has not been declared\n", node->assignStmt.varName);
            }
            semanticAnalysis(node->assignStmt.expr, symTab);
            break;  
        case NodeType_Expr:
            semanticAnalysis(node->expr.left, symTab);
            semanticAnalysis(node->expr.right, symTab);
            break;
        case NodeType_BinOp:
            // Check for variable declarations in binary operations
            if (findSymbol(symTab, node->binOp.left->varDecl.varName) == NULL) {
                fprintf(stderr, "Semantic error: Variable %s has not been declared\n", node->binOp.left->varDecl.varName);
            }
            if (findSymbol(symTab, node->binOp.right->varDecl.varName) == NULL) {
                fprintf(stderr, "Semantic error: Variable %s has not been declared\n", node->binOp.right->varDecl.varName);
            }
            semanticAnalysis(node->binOp.left, symTab);
            semanticAnalysis(node->binOp.right, symTab);
            break;
        case NodeType_LogicalOp:
            // Check logical operations
            semanticAnalysis(node->logicalOp.left, symTab);
            semanticAnalysis(node->logicalOp.right, symTab);
            break;
        case NodeType_IfStmt:
            semanticAnalysis(node->ifStmt.condition, symTab);
            semanticAnalysis(node->ifStmt.thenBlock, symTab);
            if (node->ifStmt.elseBlock != NULL) {
                semanticAnalysis(node->ifStmt.elseBlock, symTab);
            }
            break;
        case NodeType_WhileStmt:
            semanticAnalysis(node->whileStmt.condition, symTab);
            semanticAnalysis(node->whileStmt.block, symTab);
            break;
        case NodeType_ReturnStmt:
            semanticAnalysis(node->returnStmt.expr, symTab);
            break;
        case NodeType_SimpleID:
            if (findSymbol(symTab, node->simpleID.name) == NULL) {
                fprintf(stderr, "Semantic error: Variable %s has not been declared\n", node->simpleID.name);
            }
            break;
        case NodeType_SimpleExpr:
            // No checks necessary for numeric expressions
            break;
        case NodeType_WriteStmt:
            if (findSymbol(symTab, node->writeStmt.varName) == NULL) {
                fprintf(stderr, "Semantic error: Variable %s has not been declared\n", node->writeStmt.varName);
            }
            break;
        case NodeType_Block:
            semanticAnalysis(node->block.stmtList, symTab);
            break;
        // ... handle other node types ...

        default:
            fprintf(stderr, "Unknown Node Type\n");
    }

       // ... other code ...

    if (node->type == NodeType_Expr || node->type == NodeType_SimpleExpr) {
        TAC* tac = generateTACForExpr(node);
        // Process or store the generated TAC
        printTAC(tac);
    }

    // ... other code ...

}

// You can add more functions related to semantic analysis here
// Implement functions to generate TAC expressions



TAC* generateTACForExpr(ASTNode* expr) {
    // Depending on your AST structure, generate the appropriate TAC
    // If the TAC is generated successfully, append it to the global TAC list
    // Return the generated TAC, so that it can be used by the caller, e.g. for printing
    if (!expr) return NULL;

    TAC* instruction = (TAC*)malloc(sizeof(TAC));
    if (!instruction) return NULL;

    switch (expr->type) {
        case NodeType_Expr: {
            printf("Generating TAC for expression\n");
            instruction->arg1 = createOperand(expr->expr.left);
            instruction->arg2 = createOperand(expr->expr.right);
            instruction->op = strdup("+"); // Or other binary operators
            instruction->result = createTempVar();
            break;
        }
        case NodeType_SimpleExpr: {
            printf("Generating TAC for simple expression\n");
            char buffer[20];
            snprintf(buffer, 20, "%d", expr->simpleExpr.number);
            instruction->arg1 = strdup(buffer);
            instruction->op = "=";
            instruction->arg2 = NULL;
            instruction->result = createTempVar();
            break;
        }
        case NodeType_SimpleID: {
            printf("Generating TAC for simple ID\n");
            instruction->arg1 = strdup(expr->simpleID.name);
            instruction->op = strdup("assign");
            instruction->result = createTempVar();
            break;
        }
        case NodeType_AssignStmt: {
            printf("Generating TAC for assignment statement\n");
            instruction->arg1 = createOperand(expr->assignStmt.expr);
            instruction->op = strdup("=");
            instruction->result = strdup(expr->assignStmt.varName);
            break;
        }
        case NodeType_BinOp: {
            printf("Generating TAC for binary operation\n");
            instruction->arg1 = createOperand(expr->binOp.left);
            instruction->arg2 = createOperand(expr->binOp.right);
            instruction->op = strdup(&expr->binOp.operator); // Handle the operator dynamically
            instruction->result = createTempVar();
            break;
        }
        case NodeType_LogicalOp: {
            printf("Generating TAC for logical operation\n");
            instruction->arg1 = createOperand(expr->logicalOp.left);
            instruction->arg2 = createOperand(expr->logicalOp.right);
            instruction->op = strdup(expr->logicalOp.logicalOp);
            instruction->result = createTempVar();
            break;
        }
        case NodeType_WriteStmt: {
            printf("Generating TAC for WRITE statement\n");
            instruction->arg1 = strdup(expr->writeStmt.varName);
            instruction->op = strdup("write");
            instruction->result = NULL;
            break;
        }
        case NodeType_IfStmt: {
            printf("Generating TAC for IF statement\n");
            TAC* condTac = generateTACForExpr(expr->ifStmt.condition);
            appendTAC(&tacHead, condTac);

            TAC* thenTac = generateTACForExpr(expr->ifStmt.thenBlock);
            appendTAC(&tacHead, thenTac);

            if (expr->ifStmt.elseBlock != NULL) {
                TAC* elseTac = generateTACForExpr(expr->ifStmt.elseBlock);
                appendTAC(&tacHead, elseTac);
            }
            break;
        }
        case NodeType_WhileStmt: {
            printf("Generating TAC for WHILE statement\n");
            TAC* condTac = generateTACForExpr(expr->whileStmt.condition);
            appendTAC(&tacHead, condTac);

            TAC* bodyTac = generateTACForExpr(expr->whileStmt.block);
            appendTAC(&tacHead, bodyTac);
            break;
        }
        case NodeType_ReturnStmt: {
            printf("Generating TAC for RETURN statement\n");
            instruction->arg1 = createOperand(expr->returnStmt.expr);
            instruction->op = strdup("return");
            instruction->result = NULL;
            break;
        }

        default:
            free(instruction);
            return NULL;
    }

    instruction->next = NULL; // Make sure to null-terminate the new instruction

    // Append to the global TAC list
    appendTAC(&tacHead, instruction);

    return instruction;
}
// Function to create a new temporary variable for TAC
char* createTempVar() {
    static int count = 0;
    char* tempVar = malloc(10); // Enough space for "t" + number
    if (!tempVar) return NULL;
    count = allocateNextAvailableTempVar(tempVars);
    sprintf(tempVar, "t%d", count++);
    return tempVar;
}

char* createOperand(ASTNode* node) {
    // Depending on your AST structure, return the appropriate string
    // representation of the operand. For example, if the operand is a simple
    // expression or an identifier, return its string representation.
    if (!node) return NULL;

    switch (node->type) {
        case NodeType_SimpleExpr: {
            // Handle simple expressions (numeric literals)
            char* buffer = malloc(20);
            snprintf(buffer, 20, "%d", node->simpleExpr.number);
            return buffer;
        }
        case NodeType_SimpleID: {
            // Handle identifiers (variable names)
            return strdup(node->simpleID.name);
        }
        case NodeType_Expr: {
            // Handle expressions (binary or unary expressions)
            return createTempVar();  // Assuming intermediate results are stored in temporary variables
        }
        case NodeType_BinOp: {
            // Handle binary operations by creating a temporary variable to store the result
            return createTempVar();
        }
        case NodeType_LogicalOp: {
            // Handle logical operations by creating a temporary variable to store the result
            return createTempVar();
        }
        case NodeType_AssignStmt: {
            // For assignment, the operand is the variable being assigned to
            return strdup(node->assignStmt.varName);
        }
        case NodeType_WriteStmt: {
            // Handle write statements (the operand is the variable being written)
            return strdup(node->writeStmt.varName);
        }
        case NodeType_ReturnStmt: {
            // For return statements, the operand is the expression being returned
            return createOperand(node->returnStmt.expr);
        }
        case NodeType_IfStmt: {
            // If statements don't return a direct operand, so handle accordingly (optional)
            return NULL;  // Typically, you don't need an operand for if statements
        }
        case NodeType_WhileStmt: {
            // While statements also don’t return a direct operand (optional)
            return NULL;  // Similar to if statements, no direct operand needed
        }

        // Add more cases as needed for other node types...

        default:
            return NULL;
    }
}

void printTAC(TAC* tac) {
    if (!tac) return;

    // Print the TAC instruction with non-null fields
    if(tac->result != NULL)
        printf("%s = ", tac->result);
    if(tac->arg1 != NULL)
        printf("%s ", tac->arg1);
    if(tac->op != NULL)
        printf("%s ", tac->op);
    if(tac->arg2 != NULL)
        printf("%s ", tac->arg2);
    printf("\n");
}

// Print the TAC list to a file
// This function is provided for reference, you can modify it as needed

void printTACToFile(const char* filename, TAC* tac) {
    FILE* file = fopen(filename , "w");
    if (!file) {
        perror("Failed to open file");
        return;
    }   
    TAC* current = tac;
    while (current != NULL) {
        if (strcmp(current->op,"=") == 0) {
            fprintf(file, "%s = %s\n", current->result, current->arg1);
        } 
        else {
            if(current->result != NULL)
                fprintf(file, "%s = ", current->result);
            if(current->arg1 != NULL)
                fprintf(file, "%s ", current->arg1);
            if(current->op != NULL)
                fprintf(file, "%s ", current->op);
            if(current->arg2 != NULL)
                fprintf(file, "%s ", current->arg2);
            fprintf(file, "\n");
    }
        current = current->next;
    }   
    fclose(file);
    printf("TAC written to %s\n", filename);
}


// Temporary variable allocation and deallocation functions //

void initializeTempVars() {
    for (int i = 0; i < 20; i++) {
        tempVars[i] = 0;
    }
}

int allocateNextAvailableTempVar(int tempVars[]) {
   // implement the temp var allocation logic
   // use the tempVars array to keep track of allocated temp vars

    // search for the next available temp var
    for (int i = 0; i < 20; i++) {
        if (tempVars[i] == 0) {
            tempVars[i] = 1;
            return i;
        }
    }
    return -1; // No available temp var
}

void deallocateTempVar(int tempVars[], int index) {
    // implement the temp var deallocation logic
    // use the tempVars array to keep track of allocated temp vars
    if (index >= 0 && index < 20) {
        tempVars[index] = 0;
    }
}   

void appendTAC(TAC** head, TAC* newInstruction) {
    if (!*head) {
        *head = newInstruction;
    } else {
        TAC* current = *head;
        while (current->next) {
            current = current->next;
        }
        current->next = newInstruction;
    }
}
