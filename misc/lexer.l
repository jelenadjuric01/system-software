
%{
#include "parser.h"
#include "./inc/assemblerHelp.hpp"
int lineNumber=1;
%}


%option outfile="lexer.c" header-file="lexer.h"


%option noyywrap

LPAR  "["
RPAR  "]"
PLUS  "+"
MINUS "-"
SEMI  ";"
COMMA ","
ENDL  "\n"


HEX   0[Xx][0-9A-Fa-f]+
STRING \"(.+|\n+)\"
IDENT [A-Za-z_][A-Za-z0-9_]*
DEC (0|[1-9][0-9]*)
BADIDENT [0-9][A-Za-z0-9_]*
COM   "#"[^\n\r]*
WS    [ \t]*

INST_NO_OPERAND (halt|ret|iret|ret|int)
DIRECTIVE_EG \.(extern|global)[ \t]+[A-Za-z_][A-Za-z0-9_]*[ \t]*([ \t]*,[ \t]*[A-Za-z_][A-Za-z0-9_]*)*
WORD \.word[ \t]
ASCII \.ascii[ \t]
SKIP \.skip[ \t]
EQU \.equ[ \t]
SECTION \.section[ \t]
END \.end
LABEL [A-Za-z_][A-Za-z0-9_]*:
JMP_CALL (jmp|call)[ \t]
INST_OPER (push|pop|not)[ \t]
REG "%"(r(0|[1-9]|1[0-5])|sp|pc)
INST_TWO_OPER (xchg|add|sub|mul|div|and|or|xor|shl|shr)[ \t]
LD ld[ \t]
ST st[ \t]
CSRRD csrrd[ \t]
CSRWR csrwr[ \t]
SIS_REG "%"(status|handler|cause)
OPERAND_DEC \$(0|[1-9][0-9]*)
OPERAND_HEX \$0[Xx][0-9A-Fa-f]+
INST_THREE (beq|bne|bgt)[ \t]
OPERAND_S \$([A-Za-z_][A-Za-z0-9_]*)



%%
{MINUS}			{
							return TOKEN_MINUS;}
{WS}        { /* skip whitspace */ }
{END}      {lineNumber=1;return TOKEN_END;}
{OPERAND_S} {yytext++;yylval.ident = strdup(yytext); return TOKEN_S;}
{COM}       { /* also skip comments */ }
{REG}				{
	          yylval.ident = strdup(yytext);
			  return TOKEN_REG;}
{SIS_REG}   { yylval.ident = strdup(yytext);
			  return TOKEN_SIS_REG;}
{INST_TWO_OPER} {
				yylval.directive = strdup(yytext);
			  return TOKEN_INST_TWO_OPER;}
{INST_THREE} {
				yylval.directive = strdup(yytext);
			  return TOKEN_INST_THREE;}
{LD}	{return TOKEN_LD;}
{ST}	{return TOKEN_ST;}
{CSRRD}	{return TOKEN_CSRRD;}
{CSRWR}	{return TOKEN_CSRWR;}
{INST_OPER}	{
	          yylval.directive = strdup(yytext);
			  return TOKEN_INST_OPER;}
{JMP_CALL}  {
	          yylval.directive = strdup(yytext);
			  return TOKEN_JMPCALL;}
{INST_NO_OPERAND} {
	          yylval.directive = strdup(yytext);
			  return TOKEN_INST_NO;
            }
{DIRECTIVE_EG} {
				yylval.directive = strdup(yytext);
			  return TOKEN_DIRECTIVE;}
{WORD} {
			  return TOKEN_WORD;}    
{SKIP} {
			  return TOKEN_SKIP;}   
{ASCII} {
			  return TOKEN_ASCII;}  
{EQU} { 
			  return TOKEN_EQU;}     
{SECTION}   {
						return TOKEN_SECTION;}
{HEX}		{
	          sscanf(yytext, "%x", &yylval.num);
			  return TOKEN_NUM;
            }
{STRING} {
					yylval.ident = strdup(yytext);
			  return TOKEN_STRING;}
{LABEL}  {   
	          yylval.ident = strdup(yytext);
			  	return TOKEN_LABEL;
						}
{IDENT}     {
	          yylval.ident = strdup(yytext);
			  return TOKEN_IDENT;
            }

{LPAR}      { return TOKEN_LPAR; }
{RPAR}      { return TOKEN_RPAR; }
{PLUS}      { return TOKEN_PLUS; }
{SEMI}      { 
							return TOKEN_SEMI; }
{COMMA}     { 
							return TOKEN_COMMA; }
{ENDL}      {++lineNumber;
							return TOKEN_ENDL;}
{OPERAND_DEC} {yytext++;sscanf(yytext, "%d", &yylval.num); return TOKEN_LIT;}
{OPERAND_HEX} {yytext++;sscanf(yytext, "%x", &yylval.num); return TOKEN_LIT_H;}
{DEC}       {
							sscanf(yytext, "%d", &yylval.num); return TOKEN_DEC;}
{BADIDENT}  {printf("Los identifikator!\n"); return 1;}
. 				{return 1;}
%%

/* In the event of some kind of error, this will get called. */
int
yyerror(const char* msg)
{
    fprintf(stderr, "lexer error: %s on line %d\n", msg,lineNumber);
    exit(0);
}