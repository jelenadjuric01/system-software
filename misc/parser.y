
%{
	#include "./inc/assemblerHelp.hpp"
	#include <stdio.h>
	int yylex(void);
	void yyerror(const char*);
  extern int lineNumber;
  extern FILE* yyin;
  #include <stdlib.h>
  #include <string.h>
  extern int currentPass;
%}
%output "parser.c"
%defines "parser.h"

%union {
	int         num;
	char       *ident;
  char       *directive;
}

%token TOKEN_LPAR
%token TOKEN_RPAR
%token TOKEN_PLUS
%token TOKEN_SEMI
%token TOKEN_COMMA
%token TOKEN_END
%token TOKEN_ENDL
%token TOKEN_SECTION
%token TOKEN_SKIP
%token TOKEN_EQU
%token TOKEN_WORD
%token TOKEN_ASCII
%token TOKEN_MINUS
%token TOKEN_LD
%token TOKEN_ST
%token TOKEN_CSRRD
%token TOKEN_CSRWR
%token <ident> TOKEN_S


%token <num>   TOKEN_NUM
%token <ident> TOKEN_IDENT
%token <ident> TOKEN_LABEL
%token <directive> TOKEN_INST_NO
%token <directive> TOKEN_JMPCALL
%token <directive> TOKEN_INST_OPER
%token <directive> TOKEN_INST_TWO_OPER
%token <directive> TOKEN_INST_THREE
%token <ident> TOKEN_REG
%token <ident> TOKEN_SIS_REG
%token <ident> TOKEN_STRING
%token <directive> TOKEN_DIRECTIVE
%token <num>   TOKEN_DEC
%token <num>   TOKEN_LIT
%token <num>   TOKEN_LIT_H

%type <num> literal;
%type <ident> instr;

%%

input
  :prog end{
    if(currentPass==1)
    {Assembler::endFirstPass();
    }
    else{
      Assembler::endSecondPass(lineNumber);
    }
    return 0;
    
  }
  ;
prog
  : 
  | prog instr {if(currentPass==1){Assembler::instructionFirstPass(lineNumber-1);}}
  | prog section
  | prog directive
  | prog label
  | prog TOKEN_ENDL
  ;
end
  : TOKEN_END TOKEN_ENDL
  | TOKEN_END
  ;
label
  : TOKEN_LABEL { ;if(currentPass==1){ Assembler::labelFirstPass($1,lineNumber-1); }free($1);} 
  ;

instr
  : TOKEN_INST_NO TOKEN_ENDL{
    if(strcmp($1,"iret")==0 && currentPass==1){
      Assembler::iretFirstPass();
    }
    else if(currentPass==2){
      Assembler::instructionSecondPassNoPool($1,"","",lineNumber-1);
    }
    free($1);
  }
  | TOKEN_INST_OPER TOKEN_REG TOKEN_ENDL {
                                            if(currentPass==2){ Assembler::instructionSecondPassNoPool($1,$2,"",lineNumber-1);}
                                          free($1);free($2);}
  | TOKEN_JMPCALL operand TOKEN_ENDL     {Assembler::addJmp(true); if(currentPass==1){Assembler::parseInstructionFirstPass(0,lineNumber-1);} 
                      else{Assembler::instructionSecondPassPool($1,"","",lineNumber-1);}free($1);}
  | TOKEN_INST_TWO_OPER TOKEN_REG TOKEN_COMMA TOKEN_REG TOKEN_ENDL {
                        if(currentPass==2){ Assembler::instructionSecondPassNoPool($1,$2,$4,lineNumber-1);}free($2);free($1);}
  | TOKEN_LD operand_pod TOKEN_COMMA TOKEN_REG TOKEN_ENDL  { if(currentPass==1){ 
                  Assembler::parseInstructionFirstPass(1,lineNumber-1);} 
                  else{
                    Assembler::instructionSecondPassPool("ld",$4,"",lineNumber-1);
                  }
                  free($4);}
  | TOKEN_ST TOKEN_REG TOKEN_COMMA operand_pod TOKEN_ENDL  {if(currentPass==1){ 
                  Assembler::parseInstructionFirstPass(2,lineNumber-1);}
                  else{Assembler::instructionSecondPassPool("st",$2,"",lineNumber-1);} 
                  free($2);}
  | TOKEN_CSRRD TOKEN_SIS_REG TOKEN_COMMA TOKEN_REG TOKEN_ENDL  {
                    if(currentPass==2){Assembler::instructionSecondPassNoPool("csrrd",$2,$4,lineNumber-1);} free($4);free($2);}
  | TOKEN_CSRWR TOKEN_REG TOKEN_COMMA TOKEN_SIS_REG TOKEN_ENDL  { 
                    if(currentPass==2){Assembler::instructionSecondPassNoPool("csrwr",$2,$4,lineNumber-1);}free($2);free($4);}
  | TOKEN_INST_THREE TOKEN_REG TOKEN_COMMA TOKEN_REG TOKEN_COMMA operand TOKEN_ENDL {Assembler::addJmp(true);
  if(currentPass==1){ Assembler::parseInstructionFirstPass(0,lineNumber-1);}
              else{
                Assembler::instructionSecondPassPool($1,$2,$4,lineNumber-1);
              } free($1);free($2);free($4);}
  ;
operand_pod
  : TOKEN_LPAR TOKEN_REG TOKEN_PLUS operand TOKEN_RPAR {Assembler::addReg($2);Assembler::addDir(true);free($2);}
  | TOKEN_LPAR TOKEN_REG TOKEN_RPAR {Assembler::addReg($2);Assembler::addDir(true);free($2);}
  | TOKEN_S {Assembler::addSymbol($1); free($1);}
  | TOKEN_LIT {Assembler::addLiteral($1);}
  | TOKEN_LIT_H {Assembler::addLiteral($1);}
  | operand {Assembler::addDir(true);}
  | TOKEN_REG {Assembler::addReg($1); free($1);}
  ;
operand
  : literal {Assembler::addLiteral($1);}
  | TOKEN_IDENT {Assembler::addSymbol($1);free($1);}
section
  : TOKEN_SECTION TOKEN_IDENT TOKEN_ENDL{
    if(currentPass==1)
      {    Assembler::sectionFirstPass($2,lineNumber-1);
      }   
    else{
      Assembler::sectionSecondPass($2);
    } 
    free($2);
  }
  ;
directive
  : TOKEN_DIRECTIVE TOKEN_ENDL{
    if(currentPass==1)
{    Assembler::globalOrExternSymbol($1,lineNumber-1);
}    free($1);
  }
  | TOKEN_SKIP literal TOKEN_ENDL{
        if(currentPass==1)
{    Assembler::skipFirstPass($2,lineNumber-1);
}   
  else{
    Assembler::skipSecondPass($2);
  }
  }
  | TOKEN_WORD lista
  | TOKEN_ASCII TOKEN_STRING TOKEN_ENDL{
    if(currentPass==1)
{    Assembler::asciiFirstPass($2,lineNumber-1);
}    else{
    Assembler::asciiSecondPass($2);
}
    free($2);
  }
  | TOKEN_EQU TOKEN_IDENT TOKEN_COMMA lista1 {
    if(currentPass==1)
{    Assembler::equFirstPass($2,lineNumber-1);
}    else{
  Assembler::equSecondPass($2);
}
 free($2);
  }
  ;
lista1
  : TOKEN_IDENT TOKEN_PLUS lista1 {if(currentPass==1){Assembler::addToEquFirstPass($1,"+",0,false);}free($1);}
  | TOKEN_IDENT TOKEN_MINUS lista1 {if(currentPass==1){Assembler::addToEquFirstPass($1,"-",0,false);}free($1);}
  | literal TOKEN_PLUS lista1 { if(currentPass==1){Assembler::addToEquFirstPass("","+",$1,true);}}
  | literal TOKEN_MINUS lista1 { if(currentPass==1){Assembler::addToEquFirstPass("","-",$1,true);}}
  | TOKEN_IDENT TOKEN_ENDL {if(currentPass==1){Assembler::addToEquFirstPass($1,"",0,false);free($1);}}
  | literal TOKEN_ENDL {if(currentPass==1) {Assembler::addToEquFirstPass("","",$1,true);}}
lista
  : TOKEN_IDENT TOKEN_COMMA lista {if(currentPass==1){Assembler::wordFirstPass($1,lineNumber-1);} 
                                    else{Assembler::wordSecondPass($1,0,lineNumber-1);} free($1);}
  | literal TOKEN_COMMA lista {if(currentPass==1){Assembler::wordFirstPass("",lineNumber-1);}
                                  else{Assembler::wordSecondPass("",$1,lineNumber-1);}}
  | TOKEN_IDENT TOKEN_ENDL {if(currentPass==1){Assembler::wordFirstPass($1,lineNumber-1);}
                            else{Assembler::wordSecondPass($1,0,lineNumber-1);}free($1);}
  | literal TOKEN_ENDL {if(currentPass==1){Assembler::wordFirstPass("",lineNumber-1);}
                        else {Assembler::wordSecondPass("",$1,lineNumber-1);}}
literal
  : TOKEN_NUM { $$=$1;}
  | TOKEN_DEC { $$=$1;}


%%





