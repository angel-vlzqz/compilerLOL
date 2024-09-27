%option noyywrap

%{
#include <stdio.h>
#include <string.h>

#define YY_DECL int yylex()

int words = 0;
int chars = 0;
int lines = 0;

%}

letter      [a-zA-Z]
digit       [0-9]
ID          {letter}({letter}|{digit})*
delim       [ \t\n]
NUMBER         {digit}+(\.{digit}+)?(E[+\-]?(digit)+)?
ws          {delim}+

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
		}

"char"	{words++; chars += strlen(yytext);
		 printf("%s : TYPE\n", yytext);
		}

"string"	{words++; chars += strlen(yytext);
			 printf("%s : TYPE\n", yytext);
			}

"float"	{words++; chars += strlen(yytext);
		 printf("%s : TYPE\n", yytext);
		}
		
"write"	{words++; 
		 chars += strlen(yytext);
		 printf("%s : KEYWORD\n", yytext);
		}

"while"	{words++; 
		 chars += strlen(yytext);
		 printf("%s : KEYWORD\n", yytext);
		}

"return"	{words++; 
			 chars += strlen(yytext);
			 printf("%s : KEYWORD\n", yytext);
			}

"if"	{words++; 
		 chars += strlen(yytext);
		 printf("%s : KEYWORD\n", yytext);
		}

"else"	{words++; 
		 chars += strlen(yytext);
		 printf("%s : KEYWORD\n", yytext);
		}
		
{ID}	{words++; chars += strlen(yytext);
		 printf("%s : IDENTIFIER\n",yytext);
		}
			
{NUMBER}	{words++; chars += strlen(yytext);
			  printf("%s : NUMBER\n",yytext);
			}
			
";"		{chars++;
		 printf("%s : SEMICOLON\n", yytext);
		}
		
"="		{chars++;
		 printf("%s : EQ\n", yytext);
		}

"+"		{chars++;
		 printf("%s : PLUS\n", yytext);
		}

"-" 	{chars++;
		 printf("%s : MINUS\n", yytext);
		}

"*"		{chars++;
		 printf("%s : MUL\n", yytext);
		}

"=="	{words++;
		 chars += strlen(yytext);
		 printf("%s : LOGICOP\n", yytext);
		}

"&&"	{chars+= 2;
		printf("%s : LOGICOP\n", yytext);
		}
		
"||"	{chars+= 2;
		printf("%s : LOGICOP\n", yytext);
		}
		
"!"	{chars++;
		printf("%s : LOGICOP\n", yytext);
		}

"<"	{chars++;
		printf("%s : LOGICOP\n", yytext);
		}
		
"<="	{chars+= 2;
		printf("%s : LOGICOP\n", yytext);
		}
		
">"	{chars+= 2;
		printf("%s : LOGICOP\n", yytext);
		}
		
">="	{chars+= 2;
		printf("%s : LOGICOP\n", yytext);
		}
		
"!="	{chars+= 2;
		printf("%s : LOGICOP\n", yytext);
		}

"("	{chars++;
		printf("%s : OPEN\n", yytext);
		}
		
")"	{chars++;
		printf("%s : CLOSE\n", yytext);
		}
		
"["	{chars++;
		printf("%s : OPEN\n", yytext);
		}
		
"]"	{chars++;
		printf("%s : CLOSE\n", yytext);
		}
		
"{"	{chars++;
		printf("%s : OPEN\n", yytext);
		}
		
"}"	{chars++;
		printf("%s : CLOSE\n", yytext);
		}

\n		{lines++; chars=0;}
[ \t]	{chars++;}
.		{chars++;
         printf("%s : Unrecognized symbol at line %d char %d\n", yytext,lines,chars);
		}
		


%%
