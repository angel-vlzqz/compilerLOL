#include "AST.h"
int indentValue = 2;

void printIndent(int level)
{
    for (int i = 0; i < level - 1; i++)
    {
        printf("--");
    }
}

void traverseAST(ASTNode *node, int level)
{
    if (!node)
    {
        printf("Nothing to traverse\n");
        return;
    }

    printIndent(level);
    // printf("Traversing node of type %d\n", node->type);

    switch (node->type)
    {
    case NodeType_Program:
        printIndent(level);
        printf("Program\n");
        traverseAST(node->program.varDeclList, level + 1);
        traverseAST(node->program.stmtList, level + 1);
        break;
    case NodeType_VarDeclList:
        printIndent(level);
        printf("VarDeclList\n");
        traverseAST(node->varDeclList.varDecl, level + 1);
        traverseAST(node->varDeclList.varDeclList, level + 1);
        break;
    case NodeType_VarDecl:
        printIndent(level);
        printf("VarDecl: %s %s\n", node->varDecl.varType, node->varDecl.varName);
        break;
    case NodeType_SimpleExpr:
        printIndent(level);
        printf("SimpleExpr: %d\n", node->simpleExpr.number);
        printf("%d\n", node->simpleExpr.number);
        break;
    case NodeType_SimpleID:
        printIndent(level);
        printf("SimpleID: %d\n", node->simpleID.name);
        printf("%s\n", node->simpleID.name);
        break;
    case NodeType_Expr:
        printIndent(level);
        printf("Expr: %c\n", node->expr.operator);
        traverseAST(node->expr.left, level + 1);
        traverseAST(node->expr.right, level + 1);
        break;
    case NodeType_StmtList:
        printIndent(level);
        printf("StmtList\n");
        traverseAST(node->stmtList.stmt, level + 1);
        traverseAST(node->stmtList.stmtList, level + 1);
        break;
    case NodeType_AssignStmt:
        printIndent(level);
        printf("Stmt: %s = ", node->assignStmt.varName);
        traverseAST(node->assignStmt.expr, level + 1);
        break;
    case NodeType_BinOp:
        printIndent(level);
        printf("BinOp: %c\n", node->binOp.operator);
        traverseAST(node->binOp.left, level + 1);
        traverseAST(node->binOp.right, level + 1);
        break;
    case NodeType_LogicalOp:
        printIndent(level);
        printf("LogicalOp: %s\n", node->logicalOp.logicalOp);
        traverseAST(node->logicalOp.left, level + 1);
        traverseAST(node->logicalOp.right, level + 1);
        break;
    case NodeType_WriteStmt:
        printIndent(level);
        printf("Write: %s\n", node->writeStmt.varName);
        break;
    case NodeType_IfStmt:
        printIndent(level);
        printf("If Statement\n");
        traverseAST(node->ifStmt.condition, level + 1);
        traverseAST(node->ifStmt.thenBlock, level + 1);
        if (node->ifStmt.elseBlock)
        {
            traverseAST(node->ifStmt.elseBlock, level + 1);
        }
        break;
    case NodeType_WhileStmt:
        printIndent(level);
        printf("While Statement\n");
        traverseAST(node->whileStmt.condition, level + 1);
        traverseAST(node->whileStmt.block, level + 1);
        break;
    case NodeType_ReturnStmt:
        printIndent(level);
        printf("Return\n");
        traverseAST(node->returnStmt.expr, level + 1);
        break;
    case NodeType_Block:
        printIndent(level);
        printf("Block\n");
        traverseAST(node->block.stmtList, level + 1);
        break;
    }
}

void freeAST(ASTNode *node)
{
    if (!node)
        return;

    switch (node->type)
    {
    case NodeType_Program:
        freeAST(node->program.varDeclList);
        freeAST(node->program.stmtList);
        break;
    case NodeType_VarDeclList:
        freeAST(node->varDeclList.varDecl);
        freeAST(node->varDeclList.varDeclList);
        break;
    case NodeType_VarDecl:
        free(node->varDecl.varType); // Free the dynamically allocated string
        free(node->varDecl.varName); // Free the dynamically allocated string
        break;
    case NodeType_SimpleExpr:
        // No dynamic allocation, nothing to free
        break;
    case NodeType_SimpleID:
        free(node->simpleID.name); // Free the dynamically allocated string
        break;
    case NodeType_Expr:
        freeAST(node->expr.left);
        freeAST(node->expr.right);
        break;
    case NodeType_StmtList:
        freeAST(node->stmtList.stmt);
        freeAST(node->stmtList.stmtList);
        break;
    case NodeType_AssignStmt:
        free(node->assignStmt.varName);  // Free the dynamically allocated string
        free(node->assignStmt.operator); // Free the dynamically allocated string
        freeAST(node->assignStmt.expr);
        break;
    case NodeType_BinOp:
        freeAST(node->binOp.left);
        freeAST(node->binOp.right);
        break;
    case NodeType_LogicalOp:
        free(node->logicalOp.logicalOp); // Free the dynamically allocated string
        freeAST(node->logicalOp.left);
        freeAST(node->logicalOp.right);
        break;
    case NodeType_WriteStmt:
        free(node->writeStmt.varName); // Free the dynamically allocated string
        break;
    case NodeType_IfStmt:
        freeAST(node->ifStmt.condition);
        freeAST(node->ifStmt.thenBlock);
        freeAST(node->ifStmt.elseBlock);
        break;
    case NodeType_WhileStmt:
        freeAST(node->whileStmt.condition);
        freeAST(node->whileStmt.block);
        break;
    case NodeType_ReturnStmt:
        freeAST(node->returnStmt.expr);
        break;
    case NodeType_Block:
        freeAST(node->block.stmtList);
        break;
    }

    free(node); // Free the current node itself
}

ASTNode *createNode(NodeType type)
{
    ASTNode *newNode = (ASTNode *)malloc(sizeof(ASTNode));
    if (newNode == NULL)
    {
        // Handle memory allocation failure if needed
        printf("Memory allocation failed for AST node\n");
        exit(1);
        //return NULL;
    }

    newNode->type = type;

    // debugging: log the node creation
    // printf("Created AST node of type %d\n", type);

    // Initialize the node based on its type
    switch (type)
    {
    case NodeType_Program:
        newNode->program.varDeclList = NULL;
        newNode->program.stmtList = NULL;
        break;
    case NodeType_VarDeclList:
        newNode->varDeclList.varDecl = NULL;
        newNode->varDeclList.varDeclList = NULL;
        break;
    case NodeType_VarDecl:
        newNode->varDecl.varType = NULL;
        newNode->varDecl.varName = NULL;
        break;
    case NodeType_SimpleExpr:
        // Initialize the number to NULL
        newNode->simpleExpr.number = '\0';
        break;
    case NodeType_SimpleID:
        newNode->simpleID.name = NULL;
        break;
    case NodeType_Expr:
        newNode->expr.operator= '\0'; // Placeholder value
        newNode->expr.left = NULL;
        newNode->expr.right = NULL;
        break;
    case NodeType_StmtList:
        newNode->stmtList.stmt = NULL; // Example initialization
        newNode->stmtList.stmtList = NULL;
        break;
    case NodeType_AssignStmt:
        newNode->assignStmt.operator= NULL; // Example initialization
        newNode->assignStmt.varName = NULL;
        newNode->assignStmt.expr = NULL;
        break;
    case NodeType_BinOp:
        newNode->binOp.operator= '\0'; // Placeholder value
        newNode->binOp.left = NULL;
        newNode->binOp.right = NULL;
        break;
    case NodeType_LogicalOp:
        newNode->logicalOp.logicalOp = NULL;
        newNode->logicalOp.left = NULL;
        newNode->logicalOp.right = NULL;
        break;
    case NodeType_WriteStmt:
        newNode->writeStmt.varName = NULL;
        break;
    case NodeType_IfStmt:
        newNode->ifStmt.condition = NULL;
        newNode->ifStmt.thenBlock = NULL;
        newNode->ifStmt.elseBlock = NULL;
        break;
    case NodeType_WhileStmt:
        newNode->whileStmt.condition = NULL;
        newNode->whileStmt.block = NULL;
        break;
    case NodeType_ReturnStmt:
        newNode->returnStmt.expr = NULL;
        break;
    case NodeType_Block:
        newNode->block.stmtList = NULL;
        break;
    }

    return newNode;
}
