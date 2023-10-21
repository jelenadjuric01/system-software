#include <iostream>
#include <fstream>
#include "../inc/assemblerHelp.hpp"
#include <regex>
using namespace std;
#include "../lexer.h"
#include "../parser.h"

extern FILE* yyin;
int currentPass=1;
int main(int argc, const char *argv[])
{
    string inputFile;
    string outputFile;
    string output = argv[1];
  
    if (output == "-o")
    {
        inputFile = argv[3];
        outputFile = argv[2];
    }
    else
    {
        inputFile=argv[1];
        regex pattern("^(.*)(\\..*)$");

        smatch matches;
        if (regex_search(inputFile, matches, pattern)) {
        if (matches.size() == 3) {
            string file_name = matches[1].str();
            outputFile=file_name+".o";
        }
      }
    }

    Assembler::initialize(inputFile, outputFile);
 
    
    FILE* input=fopen(inputFile.c_str(),"r");
    if(!input){
        cout<<"File can not be opened!\n";
        return 1;
    }
    yyin=input;
    
    yyparse();
    
    yylex_destroy();
    fseek(input, 0, SEEK_SET);
    yyin=input;
    currentPass=2;
    yyparse();
   
    Assembler::createOutputFile();
    Assembler::createBinaryFile();
   fclose(input);
   
    
    return 0;
}