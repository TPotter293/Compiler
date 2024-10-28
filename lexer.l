%{
 //Jacob Aguilar and Tracy Potter
 //CST-405
 //09/08/2024
 //Starter code provided by Professor Artzi, modified by Jacob Aguilar and Tracy Potter

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define YY_DECL int yylex()
#include "parser.tab.h"

int words = 0;
int chars = 0;
int lines = 0;

%}

%option noyywrap
%option yylineno

letter      [a-zA-Z]
digit       [0-9]
ID          {letter}({letter}|{digit})*
delim       [ \t\n]
NUMBER      {digit}+(\.{digit}+)?(E[+\-]?(digit)+)?
ws          {delim}+

%%

%{
	// Handle multi-line comments
%}
"/*"         {
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

%{
	// Recognize keywords and types	
%}
"int"        { words++; chars += strlen(yytext);
                printf("%s : TYPE\n", yytext);
                yylval.strval = strdup(yytext);
                return TYPE;
              }

"char"       { words++; chars += strlen(yytext);
                printf("%s : TYPE\n", yytext);
                yylval.strval = strdup(yytext);
                return TYPE;
              }

"void"       { words++; chars += strlen(yytext);
                printf("%s : TYPE\n", yytext);
                yylval.strval = strdup(yytext);
                return TYPE;
              }

"write"      { words++;
                chars += strlen(yytext);
                printf("%s : KEYWORD\n", yytext);
                yylval.strval = strdup(yytext);
                return WRITE;
              }

"function"   { words++; chars += strlen(yytext);
                printf("%s : FUNCTION\n", yytext);
                yylval.strval = strdup(yytext);
                return FUNCTION;
              }

"var"        { words++; chars += strlen(yytext);
                printf("%s : VAR\n", yytext);
                yylval.strval = strdup(yytext);
                return VAR;
              }

%{
	// Control flow keywords
%}
"if"         { words++; chars += strlen(yytext);
                printf("%s : IF\n", yytext);
                yylval.strval = strdup(yytext);
                return IF;
              }

"else"       { words++; chars += strlen(yytext);
                printf("%s : ELSE\n", yytext);
                yylval.strval = strdup(yytext);
                return ELSE;
              }

"return"     { words++; chars += strlen(yytext);
                printf("%s : RETURN\n", yytext);
                yylval.strval = strdup(yytext);
                return RETURN;
              }

%{
	// Recognize identifiers
%}
{ID}         { words++; chars += strlen(yytext);
                printf("%s : IDENTIFIER\n", yytext);
                yylval.strval = strdup(yytext);
                return IDENTIFIER;
              }

%{
	// Recognize numbers
%}
{NUMBER}     { words++; chars += strlen(yytext);
                printf("%s : NUMBER\n", yytext);
                yylval.intval = atof(yytext);  // Convert to float or int as needed
                return NUMBER;
              }

%{
	// Recognize punctuation and operators
%}
";"          { chars++;
                printf("%s : SEMICOLON\n", yytext);
                return SEMICOLON;
              }

"="          { chars++;
                printf("%s : EQ\n", yytext);
                return EQ;
              }

"+"          { chars++;
                printf("%s : PLUS\n", yytext);
                return PLUS;
              }

"-"          { chars++;
                printf("%s : MINUS\n", yytext);
                return MINUS;
              }

"*"          { chars++;
                printf("%s : MULT\n", yytext);
                return MULT;
              }

"/"          { chars++;
                printf("%s : DIVIDE\n", yytext);
                return DIVIDE;
              }

","          { chars++;
                printf("%s : COMMA\n", yytext);
                return COMMA;
              }

\n {lines++; chars = 0;}
[ \t]+       { chars += yyleng; } // Count all whitespace characters

%{
	// Logical operators
%}
"&&"         { chars += 2;
                printf("%s : AND\n", yytext);
                return AND;
              }

"||"         { chars += 2;
                printf("%s : OR\n", yytext);
                return OR;
              }

"!"          { chars++;
                printf("%s : NOT\n", yytext);
                return NOT;
              }

%{
	// Parentheses, brackets, and braces
%}
"("          { chars++;
                printf("%s : LPAREN\n", yytext);
                return LPAREN;
              }

")"          { chars++;
                printf("%s : RPAREN\n", yytext);
                return RPAREN;
              }

"["          { chars++;
                printf("%s : LBRACKET\n", yytext);
                return LBRACKET;
              }

"]"          { chars++;
                printf("%s : RBRACKET\n", yytext);
                return RBRACKET;
              }

"{"          { chars++;
                printf("%s : LBRACE\n", yytext);
                return LBRACE;
              }

"}"          { chars++;
                printf("%s : RBRACE\n", yytext);
                return RBRACE;
              }

%{
	// Handle newlines and whitespace
%}
\n           { lines++; chars = 0; }
[ \t]+       { chars++; }

%{
	// Catch-all for unrecognized symbols	
%}
.            { chars++;
                printf("%s : Unrecognized symbol at line %d char %d\n", yytext, lines, chars);
              }
%%
