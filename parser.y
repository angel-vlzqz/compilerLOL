%{
#include <stdio.h>
#include <stdlib.h>
void yyerror(const char *s);
extern int yylex();
%}

%token ID ASSIGNOP ADD SEMICOLON WRITE NUMBER
%token TYPE_INT TYPE_CHAR
%left ADD

%%

Program:
    VarDecl Block
    ;

VarDecl:
    Type ID SEMICOLON VarDecl
    |
    /* empty */
    ;

Type:
    TYPE_INT
    | TYPE_CHAR
    ;

Block:
    StmtList
    ;

StmtList:
    Stmt StmtList
    | /* empty */
    ;

Stmt:
    ID ASSIGNOP Expr SEMICOLON
    | WRITE ID
    ;

Expr:
    ID ADD ID
    | ID ADD NUMBER
    | NUMBER ADD ID
    | NUMBER ADD NUMBER
    | ID
    | NUMBER
    ;

%%

void yyerror(const char *s) {
    exit(1);
}

int main() {
    printf("Enter your program:\n");
    yyparse();
    return 0;
}