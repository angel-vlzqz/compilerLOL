/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     NUMBER = 258,
     IF = 259,
     ELSE = 260,
     WHILE = 261,
     RETURN = 262,
     WRITE = 263,
     ID = 264,
     TYPE = 265,
     ASSIGNOP = 266,
     PLUS = 267,
     MINUS = 268,
     MUL = 269,
     LOGICOP = 270,
     SEMICOLON = 271,
     THEN = 272,
     DO = 273
   };
#endif
/* Tokens.  */
#define NUMBER 258
#define IF 259
#define ELSE 260
#define WHILE 261
#define RETURN 262
#define WRITE 263
#define ID 264
#define TYPE 265
#define ASSIGNOP 266
#define PLUS 267
#define MINUS 268
#define MUL 269
#define LOGICOP 270
#define SEMICOLON 271
#define THEN 272
#define DO 273




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 23 "parser.y"
{
	int number;
	char character;
	char* string;
	char* operator;
	struct ASTNode* ast;
}
/* Line 1529 of yacc.c.  */
#line 93 "parser.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

