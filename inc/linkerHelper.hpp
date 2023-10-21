#ifndef LINKER_HELP_H
#define LINKER_HELP_H
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <list>
#include <iomanip>
#include <regex>
#include <iterator>
#include <vector>

using namespace std;

class Linker
{
private:
    vector<string> filePath;
    string outputPath;

    static int currentSymbolNumber;
    static int currentSectionNumber;
    struct SymbolTableEntry
    {
        bool isSection;  
        bool isLocal; 
        bool isExtern;
        bool isDefined;
        int rnumber;   
        long long value;       
        
        string section; 
        string name;     
    };

     bool relocatable;
    map<string,long long> placeHelper;
    ofstream file;
      struct SectionMoved{
        string name;
        string file;
        unsigned long long size;
        long long startAdress;
    };
    struct RelocationTableEntry
    {
        long long offset;          
        string symbol;  
        long long addend;  
        bool isData;        
        string section; 
                
    };
      vector<string> equPar;
    map<string,map<string, SymbolTableEntry>> symbolTableOfFile;
    map<string,map<string,vector<RelocationTableEntry>>> relocationTableOfFile;
   
    struct SectionTableEntry
    {
        int rnumber;     
        string name; 
          
        bool needsPool;
        int poolSize;
        long long mappedAddress=0;
        unsigned long long size;            
        vector<long long> offsets;
        vector<char> data; 
    };
  
     map<string,vector<RelocationTableEntry>> relocationTable;
    map<string, SectionTableEntry> sectionTable;
  
  
  
    map<string,map<string, SectionTableEntry>> sectionTableOfFile;

    map<string, SymbolTableEntry> symbolTable;
   
    map<string, map<string, SectionMoved>> sectionMovedTable;
    void notRelocatable();


public:
    Linker(string,vector<string>,bool,map<string,long long>);
    
    void createOutputFile();

    static long long MEMORY_FAULT;


    void collectData();
    void connectSections();
    void connectSymbolTable();
    void connectRelocationTable();
    void connectSectionData();
    void resolveRelocation();

    void createBinaryFile();

    

    

};
#endif