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
FLOAT_NUMBER {digit}+(\.{digit}+)?([eE][+\-]?{digit}+)?
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

"'" { 
    printf("%s : CHAR_LITERAL\n", yytext);
}
\"([^\\"]|\\.)*\" {
    printf("%s : STRING_LITERAL\n", yytext);
}

"int"	{words++; chars += strlen(yytext);
		printf("%s : TYPE\n", yytext);
		yylval.string = strdup(yytext);
		return TYPE;
		}

"float"	{words++; chars += strlen(yytext);
		printf("%s : TYPE\n", yytext);
		yylval.string = strdup(yytext);
		return TYPE;
		}

"char"	{words++; chars += strlen(yytext);
		printf("%s : TYPE\n", yytext);
		yylval.string = strdup(yytext);
		return TYPE;
		}

"bool"	{words++; chars += strlen(yytext);
		printf("%s : TYPE\n", yytext);
		yylval.string = strdup(yytext);
		return TYPE;
		}

"void"	{words++; chars += strlen(yytext);
		printf("%s : Void\n", yytext);
		yylval.string = strdup(yytext);
		return VOID;
		}

"write"	{words++; 
		chars += strlen(yytext);
		printf("%s : WRITE\n", yytext);
		yylval.string = strdup(yytext);
		return WRITE;
		}

"main" {words++; 
		chars += strlen(yytext);
		printf("%s : MAIN\n", yytext);
		yylval.string = strdup(yytext);
		return MAIN;
		}

"true"	{words++; 
		chars += strlen(yytext);
		printf("%s : TRUE\n", yytext);
		yylval.string = strdup(yytext);
		return TRUE;
		}

"false"	{words++; 
		chars += strlen(yytext);
		printf("%s : FALSE\n", yytext);
		yylval.string = strdup(yytext);
		return FALSE;
		}
		
{ID}	{words++; chars += strlen(yytext);
		printf("%s : ID\n",yytext);
		yylval.string = strdup(yytext);
		return ID;
		}

{NUMBER} {words++; chars += strlen(yytext);
          printf("%s : NUMBER\n",yytext);
          if (strchr(yytext, '.') != NULL) {
              yylval.number = atof(yytext); // Use atof for floating-point numbers
          } else {
              yylval.number = atoi(yytext); // Still use atoi for integers
          }
          return NUMBER;
         }
			
{FLOAT_NUMBER} {
          words++; chars += strlen(yytext);
          printf("%s : FLOAT_NUMBER\n", yytext);
          yylval.float_number = atof(yytext); // Use atof for floating-point numbers
          return FLOAT_NUMBER;
         }
			
";"		{chars++;
		printf("%s : SEMICOLON\n", yytext);
		yylval.string = strdup(yytext);
		return SEMICOLON;
		}
		
"="		{chars++;
		printf("%s : ASSIGNOP\n", yytext);
		yylval.string = strdup(yytext);
		return ASSIGNOP;
		}

"+"		{chars++;
		printf("%s : PLUS\n", yytext);
		yylval.string = strdup(yytext);
		return PLUS;
		}

"-" 	{chars++;
		printf("%s : MINUS\n", yytext);
		yylval.string = strdup(yytext);
		return MINUS;
		}

"*"		{chars++;
		printf("%s : MUL\n", yytext);
		yylval.string = strdup(yytext);
		return MUL;
		}

"("	{chars++;
		printf("%s : '('\n", yytext);
		yylval.string = strdup(yytext);
		return '(';
		}
		
")"	{chars++;
		printf("%s : ')'\n", yytext);
		yylval.string = strdup(yytext);
		return ')';
		}

"["	{chars++;
		printf("%s : '['\n", yytext);
		yylval.string = strdup(yytext);
		return '[';
		}

"]"	{chars++;
		printf("%s : ']'\n", yytext);
		yylval.string = strdup(yytext);
		return ']';
		}

\n		{lines++; chars=0;}
[ \t]	{chars++;}
.		{chars++;
         printf("%s : Unrecognized symbol at line %d char %d\n", yytext,lines,chars);
		}

%%