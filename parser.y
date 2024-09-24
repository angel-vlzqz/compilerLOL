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
    VarDecl Block {
        printf("Parsed Program\n");
    }
    ;

VarDecl:
    Type ID SEMICOLON VarDecl {
        printf("Parsed Variable Declaration: %s\n", $2);
    }
    |
    /* empty */ {
        printf("Parsed Empty Variable Declaration\n");
    }
    ;

Type:
    TYPE_INT {
        printf("Parsed Type: int\n");
    }
    | TYPE_CHAR {
        printf("Parsed Type: char\n");
    }
    ;

Block:
    StmtList {
        printf("Parsed Block\n");
    }
    ;

StmtList:
    Stmt StmtList {
        printf("Parsed Statement List\n");
    }
    | /* empty */ {
        printf("Parsed Empty Statement List\n");
    }
    ;

Stmt:
    ID ASSIGNOP Expr SEMICOLON {
        printf("Parsed Assignment Statement: %s\n", $1);
    }
    | WRITE ID {
        printf("Parsed Write Statement: %s\n", $2);
    }
    ;

Expr:
    ID ADD ID {
        printf("Parsed Expression: %s + %s\n", $1, $3);
    }
    | ID ADD NUMBER {
        printf("Parsed Expression: %s + %s\n", $1, $3);
    }
    | NUMBER ADD ID {
        printf("Parsed Expression: %s + %s\n", $1, $3);
    }
    | NUMBER ADD NUMBER {
        printf("Parsed Expression: %s + %s\n", $1, $3);
    }
    | ID {
        printf("Parsed Identifier: %s\n", $1);
    }
    | NUMBER {
        printf("Parsed Number: %s\n", $1);
    }
    ;

%% 

void yyerror(const char *s) {
    printf("Error: %s\n", s);
    exit(1);
}

int main() {
    printf("Enter your program:\n");
    yyparse();
    return 0;
}