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
		 printf("%s : TYPE_INT\n", yytext);
		}

"char"	{words++; chars += strlen(yytext);
		 printf("%s : TYPE_CHAR\n", yytext);
		}
		
"write"	{words++; 
		 chars += strlen(yytext);
		 printf("%s : WRITE\n", yytext);
		}

"while"	{words++; 
		 chars += strlen(yytext);
		 printf("%s : WHILE\n", yytext);
		}

"return"	{words++; 
			 chars += strlen(yytext);
			 printf("%s : RETURN\n", yytext);
			}

"if"	{words++; 
		 chars += strlen(yytext);
		 printf("%s : IF\n", yytext);
		}

"else"	{words++; 
		 chars += strlen(yytext);
		 printf("%s : ELSE\n", yytext);
		}
		
{ID}	{words++; chars += strlen(yytext);
		 printf("%s : ID\n",yytext);
		}
			
{NUMBER}	{words++; chars += strlen(yytext);
			  printf("%s : NUMBER\n",yytext);
			}
			
";"		{chars++;
		 printf("%s : SEMICOLON\n", yytext);
		}
		
"="		{chars++;
		 printf("%s : ASSIGNOP\n", yytext);
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
		printf("%s : '('\n", yytext);
		}
		
")"	{chars++;
		printf("%s : ')'\n", yytext);
		}
		
"["	{chars++;
		printf("%s : '['\n", yytext);
		}
		
"]"	{chars++;
		printf("%s : ']'\n", yytext);
		}
		
"{"	{chars++;
		printf("%s : '{'\n", yytext);
		}
		
"}"	{chars++;
		printf("%s : '}'\n", yytext);
		}

\n		{lines++; chars=0;}
[ \t]	{chars++;}
.		{chars++;
         printf("%s : Unrecognized symbol at line %d char %d\n", yytext,lines,chars);
		}
		


%%
