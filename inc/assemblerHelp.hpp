#ifndef ASSEMBLER_HELP_H
#define ASSEMBLER_HELP_H
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <regex>
#include <iterator>
#include <vector>
#include <map>
#include <sstream>
#include <cmath>
#include <list>
#include <bitset>
using namespace std;

class Assembler
{
private:
    static string filePath;
    static string outputPath;
    struct RelocationTableEntry
    {
        bool isData;        
        string section; 
        long long offset;          
        string symbol; 
        long long addend;          
    };
    static int currentSymbolNumber;
    static int currentSectionNumber;
    struct SymbolTableEntry
    {
        int rnumber;   
        long long value;      
       
        bool isDefined;
        string section;  
        string name;   
         bool isSection;   
        bool isLocal; 
        bool isExtern; 
    };
    
   
    
    struct EquHelpTableEntry
    {
        
        vector<string> equals;
        string symbol;
        string type;
        string typeSymbol;
    };
    struct PoolOfLiterals{
        bool isConst;
        long long adress;
        long long value; 
        string symbol;
        PoolOfLiterals(){
            isConst=false;
            adress=-1;
            value=0;
            symbol="";
        }
    };
    static map<string,EquHelpTableEntry> helpTable;
    static map<string,vector<PoolOfLiterals>> pool;
    struct Operand{
        string symbol;
        bool isJump;
        long long literal;
        int reg;
        bool isDir;//da li se cita iz memorije ili ne
        void clear(){
            symbol="";
            isJump=isDir=false;
            literal=0;
            reg=-1;
        }

    };
    static string currentSection;
    static vector<string> equ;
    static vector<string> equPar;
    struct SectionTableEntry
    {
        unsigned long long size;            
        vector<long long> offsets; 
        int rnumber;      
        string name; 
        
        vector<char> data;  
        bool needsPool;
        int poolSize=0;
    };
    static map<string, SectionTableEntry> sectionTable;

    static map<string, SymbolTableEntry> symbolTable;
    static map<string,vector<RelocationTableEntry>> relocationTable;
    
    static list<EquHelpTableEntry> equTable;
    static long long lowLimit;
    static long long highLimit;
public:
    static unsigned long long locationCounter;

    static void parseInstructionFirstPass(int,int);
    static Operand operand;
    static void iretFirstPass();
    static void addSymbol(string);
   
    static void labelFirstPass(string,int);
    static void sectionFirstPass(string,int);
    static void sectionSecondPass(string);
    static void addToEquSecondPass(string);
    static void addToEquFirstPass(string,string,long long,bool);
    static string equCheck(EquHelpTableEntry);
    static void globalOrExternSymbol(string,int);
    static void equFirstPass(string, int);
    static void skipFirstPass(long long,int);
     static void addLiteral(long long);
    static void addReg(string);
    static void addJmp(bool);
    static void addDir(bool);
    static void initialize(string,string);
    static void skipSecondPass(long long);
    static void asciiSecondPass(string);
    static void endFirstPass();
    static void wordFirstPass(string,int);
   
    static long long decimalFromLiteral(long long);
    static void endSecondPass(int); 
    static void instructionSecondPassNoPool(string,string,string,int);
    static void instructionSecondPassPool(string,string,string,int);
    static void createOutputFile();
    static void createBinaryFile();

     static void wordSecondPass(string,long long,int);
    static void asciiFirstPass(string,int);
    static void instructionFirstPass(int);
    static void equCalculateValue();
    static void equSecondPass(string);

    

};
#endif