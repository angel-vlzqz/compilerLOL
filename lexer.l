%option noyywrap

%{
#include <stdio.h>
#include <string.h>

#define YY_DECL int yylex()
#include "parser.tab.h"

int words = 0;
int chars = 0;
int lines = 0;

%}

letter       [a-zA-Z]
digit        [0-9]
ID           {letter}({letter}|{digit})*
delim        [ \t\n]
NUMBER       {digit}+
DecimalPart   ("."{digit}+)
ExponentPart  ([eE][+\-]?{digit}+)
FLOAT_NUMBER  ({digit}+{DecimalPart}{ExponentPart}?|{digit}+{ExponentPart})
ws           {delim}+

%option yylineno
%%

"/*"    				{
    							int c;
    							while((c = input()) != 0) {
    								if(c == '*') {
    									if((c = input()) == '/')
    										break;
    									else
    										unput(c);
    								}
    							}
    						}

"/*"([^*]|[\n]|(\*+([^*/]|[\n])))*\*+"/"    { /* Skip multi-line comment */ }

"//".*                                      { /* Skip single-line comment */ }

{NUMBER} {words++; chars += strlen(yytext);
          printf("%s : NUMBER\n", yytext);
          yylval.number = atoi(yytext);
          return NUMBER;
         }

{FLOAT_NUMBER}  { 
          words++; chars += strlen(yytext);
          printf("%s : FLOAT_NUMBER\n", yytext);
          yylval.float_number = atof(yytext);
          return FLOAT_NUMBER;
        }

"="     {chars++;
        printf("%s : ASSIGNOP\n", yytext);
        yylval.operator = strdup(yytext);
        return ASSIGNOP;
        }

"+"     {chars++;
        printf("%s : '+'\n", yytext);
        return '+';
        }

"-"     {chars++;
        printf("%s : '-'\n", yytext);
        return '-';
        }

"*"     {chars++;
        printf("%s : '*'\n", yytext);
        return '*';
        }

"/"     {chars++;
        printf("%s : '/'\n", yytext);
        return '/';
        }

"&&"    { chars += 2; 
		printf("%s : AND_OP\n", yytext); 
		yylval.operator = strdup(yytext); 
		return AND_OP; }

"||"    { chars += 2; 
		printf("%s : OR_OP\n", yytext); 
		yylval.operator = strdup(yytext); 
		return OR_OP; }

"!"     { chars++;    
		printf("%s : NOT_OP\n", yytext); 
		yylval.operator = strdup(yytext); 
		return NOT_OP; }

"=="    { chars += 2; 
		printf("%s : EQ_OP\n", yytext); 
		yylval.operator = strdup(yytext); 
		return EQ_OP; }

"!="    { chars += 2; 
		printf("%s : NE_OP\n", yytext); 
		yylval.operator = strdup(yytext); 
		return NE_OP; }

"<="    { chars += 2; 
		printf("%s : LE_OP\n", yytext); 
		yylval.operator = strdup(yytext); 
		return LE_OP; }

">="    { chars += 2; 
		printf("%s : GE_OP\n", yytext); 
		yylval.operator = strdup(yytext); 
		return GE_OP; }

"<"     { chars++;    
		printf("%s : LT_OP\n", yytext); 
		yylval.operator = strdup(yytext); 
		return LT_OP; }

">"     { chars++;    
		printf("%s : GT_OP\n", yytext); 
		yylval.operator = strdup(yytext); 
		return GT_OP; }

";"		{chars++;
		printf("%s : SEMICOLON\n", yytext);
		yylval.string = strdup(yytext);
		return SEMICOLON;
		}
"("     {chars++;
        printf("%s : '('\n", yytext);
        return '(';
        }

")"     {chars++;
        printf("%s : ')'\n", yytext);
        return ')';
        }

"["     {chars++;
        printf("%s : '['\n", yytext);
        return '[';
        }

"]"     {chars++;
        printf("%s : ']'\n", yytext);
        return ']';
        }

"{"     {chars++;
        printf("%s : '{'\n", yytext);
        return '{';
        }

"}"     {chars++;
        printf("%s : '}'\n", yytext);
        return '}';
        }

","     {chars++;
        printf("%s : ','\n", yytext);
        return ',';
        }

"int"   {words++; chars += strlen(yytext);
        printf("%s : TYPE\n", yytext);
        yylval.string = strdup(yytext);
        return TYPE;
        }

"float" {words++; chars += strlen(yytext);
        printf("%s : TYPE\n", yytext);
        yylval.string = strdup(yytext);
        return TYPE;
        }

"char"  {words++; chars += strlen(yytext);
        printf("%s : TYPE\n", yytext);
        yylval.string = strdup(yytext);
        return TYPE;
        }

"bool"  {words++; chars += strlen(yytext);
        printf("%s : TYPE\n", yytext);
        yylval.string = strdup(yytext);
        return TYPE;
        }

"void"  {words++; chars += strlen(yytext);
        printf("%s : VOID\n", yytext);
        yylval.string = strdup(yytext);
        return VOID;
        }

"return" {words++; chars += strlen(yytext);
        printf("%s : RETURN\n", yytext);
        return RETURN;
        }

"write" {words++; chars += strlen(yytext);
        printf("%s : WRITE\n", yytext);
        return WRITE;
        }

"main"  {words++; chars += strlen(yytext);
        printf("%s : MAIN\n", yytext);
        yylval.string = strdup(yytext);
        return MAIN;
        }

"if"    {words++; chars += strlen(yytext);
        printf("%s : IF\n", yytext);
        return IF;
        }

"else"  {words++; chars += strlen(yytext);
        printf("%s : ELSE\n", yytext);
        return ELSE;
        }

"while" {words++; chars += strlen(yytext);
        printf("%s : WHILE\n", yytext);
        return WHILE;
        }

"do"    {words++; chars += strlen(yytext);
        printf("%s : DO\n", yytext);
        return DO;
        }

"then"  {words++; chars += strlen(yytext);
        printf("%s : THEN\n", yytext);
        return THEN;
        }

"true"  {words++; chars += strlen(yytext);
        printf("%s : TRUE\n", yytext);
        return TRUE;
        }

"false" {words++; chars += strlen(yytext);
        printf("%s : FALSE\n", yytext);
        return FALSE;
        }

{ID}    {words++; chars += strlen(yytext);
        printf("%s : ID\n", yytext);
        yylval.string = strdup(yytext);
        return ID;
        }

\n      {lines++; chars=0;}
[ \t]+  {chars += yyleng;} /* skip whitespace */
.       {chars++;
         printf("%s : Unrecognized symbol at line %d char %d\n", yytext, lines, chars);
        }

%%
