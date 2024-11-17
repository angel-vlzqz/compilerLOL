#include "AST.h"
#include <string.h>

int indentValue = 2;

void printIndent(int level)
{
    for (int i = 0; i < level - 1; i++)
    {
        printf("-");
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
    printf("Traversing node of type %d\n", node->type);

    switch (node->type)
    {
    case NodeType_Program:
        printf("Program\n");
        traverseAST(node->program.declList, level + 1);
        break;

    case NodeType_DeclList:
        printf("DeclList\n");
        traverseAST(node->declList.decl, level + 1);
        traverseAST(node->declList.next, level + 1);
        break;

    case NodeType_FuncDecl:
        printf("FuncDecl: %s %s\n", node->funcDecl.returnType, node->funcDecl.funcName);
        traverseAST(node->funcDecl.paramList, level + 1);
        traverseAST(node->funcDecl.varDeclList, level + 1);
        traverseAST(node->funcDecl.block, level + 1);
        traverseAST(node->funcDecl.returnStmt, level + 1);
        if (node->funcDecl.prevSymTab)
        {
            printf("SymbolTable was here");
        }
        if (node->funcDecl.symTab)
        {
            printf("SymbolTable was here");
        }
        break;

    case NodeType_ParamList:
        printf("ParamList\n");
        traverseAST(node->paramList.param, level + 1);
        traverseAST(node->paramList.nextParam, level + 1);
        break;

    case NodeType_Param:
        printf("Param: %s %s\n", node->param.paramType, node->param.paramName);
        break;

    case NodeType_FuncCall:
        printf("FuncCall: %s\n", node->funcCall.funcName);
        traverseAST(node->funcCall.argList, level + 1);
        break;

    case NodeType_ArgList:
        printf("ArgList\n");
        traverseAST(node->argList.arg, level + 1);
        traverseAST(node->argList.argList, level + 1);
        break;

    case NodeType_Arg:
        printf("Arg\n");
        traverseAST(node->arg.expr, level + 1);
        break;

    case NodeType_VarDeclList:
        printf("VarDeclList\n");
        traverseAST(node->varDeclList.varDecl, level + 1);
        traverseAST(node->varDeclList.varDeclList, level + 1);
        break;

    case NodeType_VarDecl:
        printf("VarDecl: %s %s\n", node->varDecl.varType, node->varDecl.varName);
        if (node->varDecl.initialValue)
        {
            printf("Initial Value:\n");
            traverseAST(node->varDecl.initialValue, level + 1);
        }
        break;

    case NodeType_ArrayDecl:
        printf("ArrayDecl: %s %s[%d]\n", node->arrayDecl.varType, node->arrayDecl.varName, node->arrayDecl.size);
        break;

    case NodeType_SimpleExpr:
        if (node->simpleExpr.isFloat)
            printf("SimpleExpr (float): %f\n", node->simpleExpr.floatValue);
        else
            printf("SimpleExpr (int): %d\n", node->simpleExpr.number);
        break;

    case NodeType_SimpleID:
        printf("SimpleID: %s\n", node->simpleID.name);
        break;

    case NodeType_BinOp:
        printf("BinOp: %c\n", node->binOp.operator);
        traverseAST(node->binOp.left, level + 1);
        traverseAST(node->binOp.right, level + 1);
        break;

    case NodeType_LogicalOp:
        printf("LogicalOp: %s\n", node->logicalOp.logicalOp);
        traverseAST(node->logicalOp.left, level + 1);
        traverseAST(node->logicalOp.right, level + 1);
        break;

    case NodeType_StmtList:
        printf("StmtList\n");
        traverseAST(node->stmtList.stmt, level + 1);
        traverseAST(node->stmtList.stmtList, level + 1);
        break;

    case NodeType_AssignStmt:
        printf("AssignStmt: %s %s \n", node->assignStmt.varName, node->assignStmt.operator);
        traverseAST(node->assignStmt.expr, level + 1);
        break;

    case NodeType_ArrayAssign:
        printf("ArrayAssign: %s[...]=...\n", node->arrayAssign.arrayName);
        traverseAST(node->arrayAssign.index, level + 1);
        traverseAST(node->arrayAssign.expr, level + 1);
        break;

    case NodeType_ArrayAccess:
        printf("ArrayAccess: %s[...]\n", node->arrayAccess.arrayName);
        traverseAST(node->arrayAccess.index, level + 1);
        break;

    case NodeType_Block:
        printf("Block\n");
        traverseAST(node->block.stmtList, level + 1);
        break;

    case NodeType_ReturnStmt:
        printf("Return Statement\n");
        traverseAST(node->returnStmt.expr, level + 1);
        break;

    case NodeType_WriteStmt:
        printf("Write Statement\n");
        traverseAST(node->writeStmt.expr, level + 1);
        break;

    case NodeType_IfStmt:
        printf("If Statement\n");
        traverseAST(node->ifStmt.condition, level + 1);
        traverseAST(node->ifStmt.thenBlock, level + 1);
        if (node->ifStmt.elseBlock)
        {
            printf("Else Block:\n");
            traverseAST(node->ifStmt.elseBlock, level + 1);
        }
        break;

    case NodeType_WhileStmt:
        printf("While Statement\n");
        traverseAST(node->whileStmt.condition, level + 1);
        traverseAST(node->whileStmt.block, level + 1);
        break;

    default:
        printf("Unknown node type: %d\n", node->type);
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
        freeAST(node->program.declList);
        break;

    case NodeType_DeclList:
        freeAST(node->declList.decl);
        freeAST(node->declList.next);
        break;

    case NodeType_FuncDecl:
        free(node->funcDecl.returnType);
        free(node->funcDecl.funcName);
        freeAST(node->funcDecl.paramList);
        freeAST(node->funcDecl.varDeclList);
        freeAST(node->funcDecl.block);
        freeAST(node->funcDecl.returnStmt);
        break;

    case NodeType_ParamList:
        freeAST(node->paramList.param);
        freeAST(node->paramList.nextParam);
        break;

    case NodeType_Param:
        free(node->param.paramType);
        free(node->param.paramName);
        break;

    case NodeType_FuncCall:
        free(node->funcCall.funcName);
        freeAST(node->funcCall.argList);
        break;

    case NodeType_ArgList:
        freeAST(node->argList.arg);
        freeAST(node->argList.argList);
        break;

    case NodeType_Arg:
        freeAST(node->arg.expr);
        break;

    case NodeType_VarDeclList:
        freeAST(node->varDeclList.varDecl);
        freeAST(node->varDeclList.varDeclList);
        break;

    case NodeType_VarDecl:
        free(node->varDecl.varType);
        free(node->varDecl.varName);
        freeAST(node->varDecl.initialValue);
        break;

    case NodeType_ArrayDecl:
        free(node->arrayDecl.varType);
        free(node->arrayDecl.varName);
        break;

    case NodeType_SimpleExpr:
        // No dynamic allocation, nothing to free
        break;

    case NodeType_SimpleID:
        free(node->simpleID.name);
        break;

    case NodeType_BinOp:
        freeAST(node->binOp.left);
        freeAST(node->binOp.right);
        break;

    case NodeType_LogicalOp:
        free(node->logicalOp.logicalOp);
        freeAST(node->logicalOp.left);
        freeAST(node->logicalOp.right);
        break;

    case NodeType_StmtList:
        freeAST(node->stmtList.stmt);
        freeAST(node->stmtList.stmtList);
        break;

    case NodeType_AssignStmt:
        free(node->assignStmt.operator);
        free(node->assignStmt.varName);
        freeAST(node->assignStmt.expr);
        break;

    case NodeType_ArrayAssign:
        free(node->arrayAssign.arrayName);
        freeAST(node->arrayAssign.index);
        freeAST(node->arrayAssign.expr);
        break;

    case NodeType_ArrayAccess:
        free(node->arrayAccess.arrayName);
        freeAST(node->arrayAccess.index);
        break;

    case NodeType_Block:
        freeAST(node->block.stmtList);
        break;

    case NodeType_ReturnStmt:
        freeAST(node->returnStmt.expr);
        break;

    case NodeType_WriteStmt:
        freeAST(node->writeStmt.expr);
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

    default:
        printf("Unknown node type: %d\n", node->type);
        break;
    }

    free(node);
}

ASTNode *createNode(NodeType type)
{
    ASTNode *newNode = (ASTNode *)malloc(sizeof(ASTNode));
    if (newNode == NULL)
    {
        printf("Memory allocation failed for AST node\n");
        exit(1);
    }

    newNode->type = type;
    newNode->dataType = NULL;

    switch (type)
    {
    case NodeType_Program:
        newNode->program.declList = NULL;
        break;

    case NodeType_DeclList:
        newNode->declList.decl = NULL;
        newNode->declList.next = NULL;
        break;

    case NodeType_FuncDecl:
        newNode->funcDecl.returnType = NULL;
        newNode->funcDecl.funcName = NULL;
        newNode->funcDecl.paramList = NULL;
        newNode->funcDecl.varDeclList = NULL;
        newNode->funcDecl.block = NULL;
        newNode->funcDecl.returnStmt = NULL;
        newNode->funcDecl.prevSymTab = NULL;
        newNode->funcDecl.symTab = NULL;
        break;

    case NodeType_ParamList:
        newNode->paramList.param = NULL;
        newNode->paramList.nextParam = NULL;
        break;

    case NodeType_Param:
        newNode->param.paramType = NULL;
        newNode->param.paramName = NULL;
        break;

    case NodeType_FuncCall:
        newNode->funcCall.funcName = NULL;
        newNode->funcCall.argList = NULL;
        break;

    case NodeType_ArgList:
        newNode->argList.arg = NULL;
        newNode->argList.argList = NULL;
        break;

    case NodeType_Arg:
        newNode->arg.expr = NULL;
        break;

    case NodeType_VarDeclList:
        newNode->varDeclList.varDecl = NULL;
        newNode->varDeclList.varDeclList = NULL;
        break;

    case NodeType_VarDecl:
        newNode->varDecl.varType = NULL;
        newNode->varDecl.varName = NULL;
        newNode->varDecl.initialValue = NULL;
        newNode->varDecl.isFloat = false;
        break;

    case NodeType_ArrayDecl:
        newNode->arrayDecl.varType = NULL;
        newNode->arrayDecl.varName = NULL;
        newNode->arrayDecl.size = 0;
        newNode->arrayDecl.isFloat = false;
        break;

    case NodeType_SimpleExpr:
        newNode->simpleExpr.number = 0;
        newNode->simpleExpr.floatValue = 0.0;
        newNode->simpleExpr.isFloat = false;
        break;

    case NodeType_SimpleID:
        newNode->simpleID.name = NULL;
        break;

    case NodeType_BinOp:
        newNode->binOp.operator= '\0';
        newNode->binOp.left = NULL;
        newNode->binOp.right = NULL;
        break;

    case NodeType_LogicalOp:
        newNode->logicalOp.logicalOp = NULL;
        newNode->logicalOp.left = NULL;
        newNode->logicalOp.right = NULL;
        break;

    case NodeType_StmtList:
        newNode->stmtList.stmt = NULL;
        newNode->stmtList.stmtList = NULL;
        break;

    case NodeType_AssignStmt:
        newNode->assignStmt.operator= NULL;
        newNode->assignStmt.varName = NULL;
        newNode->assignStmt.expr = NULL;
        break;

    case NodeType_ArrayAssign:
        newNode->arrayAssign.arrayName = NULL;
        newNode->arrayAssign.index = NULL;
        newNode->arrayAssign.expr = NULL;
        break;

    case NodeType_ArrayAccess:
        newNode->arrayAccess.arrayName = NULL;
        newNode->arrayAccess.index = NULL;
        break;

    case NodeType_Block:
        newNode->block.stmtList = NULL;
        break;

    case NodeType_ReturnStmt:
        newNode->returnStmt.expr = NULL;
        break;

    case NodeType_WriteStmt:
        newNode->writeStmt.expr = NULL;
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

    default:
        printf("Unknown node type: %d\n", type);
        break;
    }

    return newNode;
}
