#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <regex>
#include <iterator>
#include <vector>
#include <map>


#include "../inc/linkerHelper.hpp"
using namespace std;
long long Linker::MEMORY_FAULT=0xFFFFFF00;
Linker::Linker(string output,vector<string> inputs,bool rel,map<string,long long> pH):
outputPath(output),filePath(inputs),relocatable(rel),placeHelper(pH),file("helpFile.txt"){}

void Linker::collectData()
{

    for (string inputFile : filePath)
    {

        ifstream input(inputFile, ios::binary);
        if (input.fail())
        {
            cout<<"Input file "<<inputFile<<" can not be opened!"<<endl;
            exit(1);
        }
    map<string,SymbolTableEntry> stable;
    int numOfSymbols = 0;
    input.read((char *)&numOfSymbols, sizeof(numOfSymbols));
    for (int i=0;i<numOfSymbols;i++)
    {
        SymbolTableEntry sym;
        string name;        
        unsigned int stringLength;
        input.read((char *)(&stringLength), sizeof(stringLength));
        name.resize(stringLength);
        input.read((char*)name.c_str(), stringLength);
        input.read((char *)(&sym.rnumber), sizeof(sym.rnumber));
        input.read((char *)(&sym.isDefined), sizeof(sym.isDefined));
        input.read((char *)(&sym.isExtern), sizeof(sym.isExtern));
        input.read((char *)(&sym.isLocal), sizeof(sym.isLocal));
        input.read((char *)(&sym.isSection), sizeof(sym.isSection));
        input.read((char *)(&sym.value), sizeof(sym.value));

        stringLength;
        input.read((char *)(&stringLength), sizeof(stringLength));
        sym.name.resize(stringLength);
        input.read((char*)sym.name.c_str(), stringLength);

        stringLength;
        input.read((char *)(&stringLength), sizeof(stringLength));
        sym.section.resize(stringLength);
        input.read((char*)sym.section.c_str(), stringLength);

        stable[name]=sym;
    }
    symbolTableOfFile[inputFile]=stable;

    map<string,SectionTableEntry> sectable;
    int numOfSections = 0;
    input.read((char *)&numOfSections, sizeof(numOfSections));

    for (int i=0;i<numOfSections;i++)
    {
        SectionTableEntry sec;
        string name;
        unsigned int stringLength;
        input.read((char *)(&stringLength), sizeof(stringLength));
        name.resize(stringLength);
        input.read((char*)name.c_str(), stringLength);
    
        input.read((char *)(&sec.rnumber), sizeof(sec.rnumber));
        input.read((char *)(&sec.needsPool), sizeof(sec.needsPool));
        input.read((char *)(&sec.poolSize), sizeof(sec.poolSize));

        input.read((char *)(&sec.size), sizeof(sec.size));

        stringLength;
        input.read((char *)(&stringLength), sizeof(stringLength));
        sec.name.resize(stringLength);
        input.read((char*)sec.name.c_str(), stringLength);
        int numOfRelocations;

        input.read((char *)&numOfRelocations, sizeof(numOfRelocations));
        vector<RelocationTableEntry>reltable;
        for (int j=0;j<numOfRelocations;j++)
        {

            RelocationTableEntry rel;

        input.read((char *)(&rel.addend), sizeof(rel.addend));
        input.read((char *)(&rel.isData), sizeof(rel.isData));
        input.read((char *)(&rel.offset), sizeof(rel.offset));

        unsigned int stringLength; 
        input.read((char *)(&stringLength), sizeof(stringLength));
        rel.section.resize(stringLength);
        input.read((char*)rel.section.c_str(), stringLength);

        stringLength;
        input.read((char *)(&stringLength), sizeof(stringLength));
        rel.symbol.resize(stringLength);
        input.read((char*)rel.symbol.c_str(), stringLength);
        reltable.push_back(rel);
    
    }
    
    relocationTableOfFile[sec.name][inputFile]=reltable;
        int numOfChars;
        input.read((char *)&numOfChars, sizeof(numOfChars));

      
        for (int k=0;k<numOfChars;k++)
        {
            char c;
            input.read((char *)(&c), sizeof(c));
            sec.data.push_back(c);
        }

        int numOfOffsets; 
        input.read((char *)(&numOfOffsets), sizeof(numOfOffsets));

    
        for (int l=0;l<numOfOffsets;l++)
        {
            long long x;
            input.read((char *)(&x), sizeof(x));
            sec.offsets.push_back(x);
        }
        sectable[name]=sec;
    }
    sectionTableOfFile[inputFile]=sectable;
    input.close();
    }
}
void Linker::createOutputFile(){
    for(string inputFile:filePath){

    file << "Relocative object file "<<inputFile << endl
                     << endl;

    file << "Section table:" << endl;
    file << "Id\tName\t\tSize\t" << endl;
    for (map<string, SectionTableEntry>::iterator it = sectionTableOfFile[inputFile].begin(); it != sectionTableOfFile[inputFile].end(); it++)
    {
        file <<hex<< (it->second.rnumber&0xf) << "\t\t" << it->second.name << "\t\t\t" << hex << setfill('0') << setw(8) << (0xffffffff & it->second.size) << endl;
    }
    file << dec;
    file << endl
                     << endl;

    file << "Symbol table:" << endl;
    file << "Value\t\tType\tSection\t\tId\tName" << endl;
    for (map<string, SymbolTableEntry>::iterator it = symbolTableOfFile[inputFile].begin(); it != symbolTableOfFile[inputFile].end(); it++)
    {
        file << hex << setfill('0') << setw(8) << (0xffffffff & it->second.value) << "\t";
        if (it->second.isLocal == true)
            file << "l\t\t";
        else
        {
            if (it->second.isDefined == true)
                file << "g\t\t";
            else
            {
                if (it->second.isExtern)
                    file << "e\t\t";
               
            }
        }
        file << it->second.section << "\t\t\t" << hex << setfill('0') << setw(4) << (0xffff & it->second.rnumber) <<"\t" << it->second.name << endl;
    }
    file << dec;
    file<< endl
                     << endl;
    for (map<string, SectionTableEntry>::iterator it = sectionTableOfFile[inputFile].begin(); it != sectionTableOfFile[inputFile].end(); it++)
    {
        file << "Relocation data <" << it->first << ">:" << endl;
        file<< "Offset\t\tDat/Equ\tSection\tAddend\tSymbol" << endl;
        
        for (RelocationTableEntry rel : relocationTableOfFile[it->first][inputFile])
        {
            
            file << hex << setfill('0') << setw(8) << (0xffffffff & rel.offset) << "\t" << (rel.isData ? 'd' : 'e') << "\t\t\t\t"  << rel.section << "\t\t\t"<<hex<<setfill('0')<<setw(4)<< (rel.addend&0xffff)<<"\t"<< rel.symbol << "\t"<<endl;
        }
        file << dec << endl;

        file << "Section data <" << it->first << ">:" << endl;

        SectionTableEntry section = it->second;
        if (section.size == 0)
        {
            file << dec;
            file << endl
                             << endl;
            continue;
        }
        int counter = 0;

        for (int i = 0; i < section.offsets.size() - 1; i++)
        {

            long long currOffset = section.offsets[i];
            long long nextOffset = section.offsets[i + 1];
            file << hex << setfill('0') << setw(8) << (0xffffffff & currOffset) << ": ";
            for (long long j = currOffset; j < nextOffset; j++)
            {
                char c = section.data[j];
                file << hex << setfill('0') << setw(2) << (0xff & c) << " ";
            }
            file << endl;
        }
        long long currOffset = section.offsets[section.offsets.size() - 1];
        long long nextOffset = section.data.size();
        file << hex << setfill('0') << setw(8) << (0xffffffff & currOffset) << ": ";
        for (long long j = currOffset; j < nextOffset; j++)
        {
            char c = section.data[j];
            file << hex << setfill('0') << setw(2) << (0xff & c) << " ";
        }
        file << dec;
        file << endl
                         << endl;
    }
    }
    
    file << "Section table:" << endl;
    file << "Id\tName\t\tSize\tStart adress" << endl;
    for (map<string, SectionTableEntry>::iterator it = sectionTable.begin(); it != sectionTable.end(); it++)
    {
        file <<hex<< (it->second.rnumber&0xf) << "\t\t" << it->second.name << "\t\t\t" << hex << setfill('0') << setw(8) << (0xffffffff & it->second.size) <<"\t"<< hex << setfill('0') << setw(8) << (0xffffffff & it->second.mappedAddress)<< endl;    }
    file << dec;
    file << endl
                     << endl;
    file << "Symbol table:" << endl;
    file << "Value\t\tType\tSection\t\tId\tName" << endl;
    for (map<string, SymbolTableEntry>::iterator it = symbolTable.begin(); it != symbolTable.end(); it++)
    {
        file << hex << setfill('0') << setw(8) << (0xffffffff & it->second.value) << "\t";
        
        if (it->second.isLocal == true)
            file << "l\t\t";
        else
        {
            if (it->second.isDefined == true)
                file << "g\t\t";
            else
            {
                if (it->second.isExtern)
                    file << "e\t\t";
               
            }
        }
        file << it->second.section << "\t\t\t" << hex << setfill('0') << setw(4) << (0xffff & it->second.rnumber) <<"\t" << it->second.name << endl;
    }
    file << dec;
    file<< endl
                     << endl;
    for (map<string, SectionTableEntry>::iterator it = sectionTable.begin(); it != sectionTable.end(); it++)
    {
        file << "Relocation data <" << it->first << ">:" << endl;
        file<< "Offset\t\tDat/Equ\tSection\tAddend\tSymbol" << endl;
        
        for (RelocationTableEntry rel : relocationTable[it->first])
        {
            
            file << hex << setfill('0') << setw(8) << (0xffffffff & rel.offset) << "\t" << (rel.isData ? 'd' : 'e') << "\t\t\t\t"  << rel.section << "\t\t\t"<<hex<<setfill('0')<<setw(4)<< (rel.addend&0xffff)<<"\t"<< rel.symbol << "\t"<<endl;
        }
        file << dec << endl;

        file << "Section data <" << it->first << ">:" << endl;

        SectionTableEntry section = it->second;
        if (section.size == 0)
        {
            file << dec;
            file << endl
                             << endl;
            continue;
        }
        int counter = 0;

        for (int i = 0; i < section.offsets.size() - 1; i++)
        {

            long long currOffset = section.offsets[i];
            long long nextOffset = section.offsets[i + 1];
            file << hex << setfill('0') << setw(8) << (0xffffffff & currOffset) << ": ";
            for (long long j = currOffset-section.mappedAddress; j < nextOffset-section.mappedAddress; j++)
            {
                char c = section.data[j];
                file << hex << setfill('0') << setw(2) << (0xff & c) << " ";
            }
            file << endl;
        }
        long long currOffset = section.offsets[section.offsets.size() - 1]-section.mappedAddress;
        long long nextOffset = section.data.size();
        file << hex << setfill('0') << setw(8) << (0xffffffff & (currOffset+section.mappedAddress)) << ": ";
        for (long long j = currOffset; j < nextOffset; j++)
        {
            char c = section.data[j];
            file << hex << setfill('0') << setw(2) << (0xff & c) << " ";
        }
        file << dec;
        file << endl
                         << endl;
    }
    
    if(!relocatable){
        file << "Content:" << endl;
        for (map<string, SectionTableEntry>::iterator it = sectionTable.begin(); it != sectionTable.end(); it++)
        {
           
            file << "Section data <" << it->first << ">:" << endl;

            SectionTableEntry sec = it->second;
            int counter = 0;

            for (int i = 0; i < sec.data.size(); i++)
            {
                char c = sec.data[i];
                if (counter % 8 == 0)
                {
                    file << hex << setfill('0') << setw(8) << (0xffffffff & counter + sec.mappedAddress) << "   ";
                }
                file << hex << setfill('0') << setw(2) << (0xff & c) << " ";
                counter++;
                if (counter % 8 == 0)
                {
                    file << endl;
                }
            }
            file << endl;

            file << dec;
            file << endl;
        }
    
    }

    file.close();
}

void Linker::notRelocatable()
{
    

    long long nextAdress = 0;
    for (map<string, long long>::iterator iter = placeHelper.begin();iter != placeHelper.end(); iter++)
    {
        sectionTable.find(iter->first)->second.mappedAddress = iter->second;

        for (map<string, SectionMoved>::iterator itSecMoved = sectionMovedTable[iter->first].begin();
             itSecMoved != sectionMovedTable[iter->first].end(); itSecMoved++)
        {
            itSecMoved->second.startAdress += iter->second;
            if (itSecMoved->second.startAdress + itSecMoved->second.size > nextAdress)
            {
                nextAdress = itSecMoved->second.startAdress + itSecMoved->second.size;
            }
        }
    }
    for (map<string, long long>::iterator iter = placeHelper.begin(); iter != placeHelper.end(); iter++){
        SectionTableEntry firstSection=sectionTable[iter->first];
        if (firstSection.mappedAddress+firstSection.size >MEMORY_FAULT)
        {
            cout<<"Section " + firstSection.name + " is going into memory mapped for registers!"<<endl;
            exit(1);
        }
        for(map<string, long long>::iterator it = iter; it != placeHelper.end(); it++){
            if(it==iter) continue;
            SectionTableEntry secondSection=sectionTable[it->first];
            if((secondSection.mappedAddress<(firstSection.mappedAddress+firstSection.size)&& firstSection.mappedAddress<secondSection.mappedAddress)
            || (firstSection.mappedAddress<(secondSection.mappedAddress+secondSection.size)&& secondSection.mappedAddress<firstSection.mappedAddress)){
                cout<< "Sections "<<firstSection.name<<" and "<<secondSection.name<<" are intersectioning!"<<endl;
                exit(1);
            }

        }
    }


    for (map<string, SectionTableEntry>::iterator iter = sectionTable.begin();iter != sectionTable.end(); iter++)
    {

        map<string, long long>::iterator it = placeHelper.find(iter->first);
        if (it == placeHelper.end() && iter->first!="UND" && iter->first!="ABS")
        {
            iter->second.mappedAddress = nextAdress;
            for (map<string, SectionMoved>::iterator itSecM = sectionMovedTable[iter->first].begin();itSecM != sectionMovedTable[iter->first].end(); itSecM++)
            {
                itSecM->second.startAdress += nextAdress;
            }
            nextAdress += iter->second.size;
        }
        if (nextAdress > MEMORY_FAULT)
        {
            cout<< "Section " << iter->second.name << " is going into memory reserved for registers!"<<endl;
            exit(1);
        }
    }
   

}

void Linker::connectSections()
{

    map<string, long long> previous_end;
    for (string filename : filePath)
    {
        for (map<string, SectionTableEntry>::iterator it = sectionTableOfFile[filename].begin(); it != sectionTableOfFile[filename].end(); it++)
        {
            previous_end[it->second.name] = 0;
        }
    }

    for (string filename : filePath)
    {


        for (map<string, SectionTableEntry>::iterator it = sectionTableOfFile[filename].begin(); it != sectionTableOfFile[filename].end(); it++)
        {
            if (it->second.name == "UND" || it->second.name == "ABS")
            {
                continue;
            }
            SectionMoved data;
            data.file = filename; 
            data.size = (it->second.needsPool?it->second.data.size():it->second.size);
            data.name = it->second.name;
            data.startAdress = previous_end[it->second.name];
            previous_end[it->second.name] = previous_end[it->second.name] + data.size;

            sectionMovedTable[it->second.name][filename] = data;
        }
    }

    int idSec = 1;
    for (map<string, long long>::iterator it = previous_end.begin();
         it != previous_end.end(); it++)
    {
        SectionTableEntry next;
        next.name = it->first;
        if (it->first == "UND")
        {
            next.rnumber = 0;
        }
        else if (it->first == "ABS")
        {
            next.rnumber = -1;
        }
        else
        {
            next.rnumber = idSec++;
        }
        next.size = it->second;
        next.mappedAddress = 0; 
        next.needsPool=false;
        next.poolSize=0;
        
        sectionTable[it->first] = next;
    }

    if (!relocatable)
    {
        notRelocatable();
        
    }
}
void Linker::resolveRelocation()
{
    if(relocatable) return;
    for(map<string,SectionTableEntry>::iterator iter=sectionTable.begin();iter!=sectionTable.end();iter++){
        for(RelocationTableEntry rel:relocationTable[iter->first]){
            if(rel.isData){
                long long value=symbolTable[rel.symbol].value+rel.addend;
                long long i=rel.offset-iter->second.mappedAddress;
                iter->second.data[i++]=(0xff&value);
                iter->second.data[i++]=(0xff&(value>>8));
                 iter->second.data[i++]=(0xff&(value>>16));
                iter->second.data[i++]=(0xff&(value>>24));
               
            }
        }
    }
}

void Linker::connectSymbolTable()
{
    int symbolId = 0;
    for (map<string, SectionTableEntry>::iterator it = sectionTable.begin();it != sectionTable.end(); it++)
    {
        SymbolTableEntry sym;
        sym.rnumber = it->second.rnumber;
        if (sym.rnumber > symbolId)
        {
            symbolId = sym.rnumber;
        }
        sym.isDefined = true;
        sym.isExtern = false;
        sym.isLocal = true;
        sym.isSection=true;
        sym.name = it->second.name;
        sym.section = it->second.name;
        sym.value = it->second.mappedAddress;
        symbolTable[sym.name] = sym;
    }
    symbolId++;
    for(string filename:filePath){
        for(map<string,SymbolTableEntry>::iterator iter=symbolTableOfFile[filename].begin();iter!=symbolTableOfFile[filename].end();iter++){
            SymbolTableEntry sym=iter->second;
            bool equ=false;
            if(relocationTableOfFile.count("ABS")>0 && (relocationTableOfFile["ABS"]).count(filename)>0){
                for(RelocationTableEntry rel: relocationTableOfFile["ABS"][filename]){
                    if(rel.offset==sym.rnumber){
                        equ=true;
                        break;
                    }
                }
            }
            if(sym.isSection || equ || sym.isLocal) continue;
            map<string,SymbolTableEntry>::iterator itSym=symbolTable.find(sym.name);
            if(itSym!=symbolTable.end()){
                if(!itSym->second.isExtern && !sym.isExtern){
                    cout<<"Symbol "<<sym.name<<" is defined multiple times!"<<endl;
                    exit(1);
                }
                else{//ne moze ovako, sta ako je prvo defined pa posle extern
                    
                    itSym->second.isDefined=true;
                    itSym->second.section=(itSym->second.isExtern?sym.section:itSym->second.section);
                    itSym->second.value=(itSym->second.isExtern?sym.value+sectionMovedTable[sym.section][filename].startAdress:itSym->second.value);
                    itSym->second.isExtern=false;

                }
            }
            else{
            SymbolTableEntry newSym;
            newSym.rnumber=symbolId++;
            newSym.isSection=false;
            newSym.isLocal=sym.isLocal;
            newSym.name=sym.name;
            if(sym.section=="ABS"){
                newSym.isDefined=sym.isDefined;
                newSym.isExtern=false;
                newSym.section=sym.section;
                newSym.value=sym.value;
            }
            else if(!sym.isExtern){
                newSym.isDefined=sym.isDefined;
                newSym.isExtern=false;
                newSym.section=sym.section;
                newSym.value=sym.value+sectionMovedTable[sym.section][filename].startAdress;
            }
            else{
                newSym.isDefined=sym.isDefined;
                newSym.isExtern=true;
                newSym.section="UND";
                newSym.value=0;
            }
            symbolTable[sym.name]=newSym;
            }
        }
    }

   for(string filename:filePath){
        for(map<string,SymbolTableEntry>::iterator iter=symbolTableOfFile[filename].begin();iter!=symbolTableOfFile[filename].end();iter++){
            {
                 SymbolTableEntry sym=iter->second;
                 
                if(relocationTableOfFile.count("ABS")>0 && (relocationTableOfFile["ABS"]).count(filename)>0){
                    for(RelocationTableEntry rel: relocationTableOfFile["ABS"][filename]){
                        if(rel.offset==sym.rnumber){
                            SymbolTableEntry newSym;
                              newSym.rnumber=symbolId++;
                            newSym.isSection=false;
                              newSym.isLocal=sym.isLocal;
                             newSym.name=sym.name;
                             newSym.isExtern=false;
                             newSym.isDefined=true;
                             newSym.section="ABS";
                             SymbolTableEntry relSym=symbolTable[rel.symbol];
                             if(!relSym.isDefined && relocatable==false){
                                cout<<"Symbol "<<rel.symbol<<" is unresovled!"<<endl;
                                exit(1);
                             }
                             if(relocatable && !relSym.isDefined){
                                newSym.isDefined=false;
                             }
                             newSym.value=(relSym.isSection?sectionMovedTable[rel.symbol][filename].startAdress:relSym.value)+rel.addend;
                             symbolTable[sym.name]=newSym;
                    }
                }
            }
            }
   }
}
    for(map<string,SymbolTableEntry>::iterator it=symbolTable.begin();it!=symbolTable.end() && relocatable==false;it++){
        if(!it->second.isDefined){
            cout<<"Symbol "<<it->second.name<<" is unresolved!"<<endl;
            exit(1);
        }
    }
}

void Linker::connectRelocationTable()
{
    
    for (string filename : filePath)
    {
        for(map<string,SectionTableEntry>::iterator it=sectionTableOfFile[filename].begin();it!=sectionTableOfFile[filename].end();it++)
        {vector<RelocationTableEntry> relTable = relocationTableOfFile[it->second.name][filename];

        for (RelocationTableEntry rel : relTable)
        {
            RelocationTableEntry relFinal;
            if(rel.isData){
            long long jump=symbolTable[rel.symbol].isSection?sectionMovedTable[rel.symbol][filename].startAdress-sectionTable[rel.symbol].mappedAddress:0;
            relFinal.addend =rel.addend+jump;
            relFinal.isData = rel.isData;
            relFinal.offset = rel.offset + sectionMovedTable[rel.section][filename].startAdress;

            relFinal.section = rel.section;
            relFinal.symbol = rel.symbol;
            }
            else{
                relFinal.isData=rel.isData;
                relFinal.symbol=rel.symbol;
                long long jump=symbolTable[rel.symbol].isSection?sectionMovedTable[rel.symbol][filename].startAdress-sectionTable[rel.symbol].mappedAddress:0;
                relFinal.addend =rel.addend+jump;
                for(map<string,SymbolTableEntry>::iterator iter=symbolTableOfFile[filename].begin();iter!=symbolTableOfFile[filename].end();iter++){
                    if(rel.offset==iter->second.rnumber){
                        relFinal.offset=symbolTable[iter->second.name].rnumber;
                        break;
                    }
                }
                relFinal.section=rel.section;
            }
            relocationTable[rel.section].push_back(relFinal);
        }
    }}
}

void Linker::connectSectionData()
{
    for (map<string, SectionTableEntry>::iterator it = sectionTable.begin(); it != sectionTable.end(); it++)
    {
        string section_name = it->first;
        if (it->second.size == 0)
        {
            continue;
        }

        for (string filename : filePath)
        {
            map<string, SectionMoved>::iterator itSec = sectionMovedTable[it->first].find(filename);
            if (itSec == sectionMovedTable[it->first].end())
            {
                continue;
            }

            SectionMoved secMoved = itSec->second;
            vector<long long> offsets = sectionTableOfFile[filename][section_name].offsets;
            vector<char> data = sectionTableOfFile[filename][section_name].data;

            for (int i = 0; i < offsets.size() - 1; i++)
            {
                long long curr = offsets[i];
                long long next = offsets[i + 1];
                it->second.offsets.push_back(curr + sectionMovedTable[section_name][filename].startAdress);
                for (long long j = curr; j < next; j++)
                {
                    char c = data[j];
                    it->second.data.push_back(c);
                }
            }
            long long curr = offsets[offsets.size() - 1];
            long long next = data.size();
            it->second.offsets.push_back(curr + sectionMovedTable[section_name][filename].startAdress);
            for (long long j = curr; j < next; j++)
            {
                char c = data[j];
                it->second.data.push_back(c);
            }
            
        }
    }
}




void Linker::createBinaryFile()
{
    if (relocatable)
    {
        ofstream binaryFile(outputPath, ios::out | ios::binary);
        int numOfSym = symbolTable.size();
        binaryFile.write((char *)&numOfSym, sizeof(numOfSym));
    for (map<string, SymbolTableEntry>::iterator it = symbolTable.begin(); it != symbolTable.end(); it++)
    {
        string key = it->first;
        
        unsigned int stringLength = key.length();
        binaryFile.write((char *)(&stringLength), sizeof(stringLength));
        binaryFile.write(key.c_str(), key.length());
        binaryFile.write((char *)(&it->second.rnumber), sizeof(it->second.rnumber));
        binaryFile.write((char *)(&it->second.isDefined), sizeof(it->second.isDefined));
        binaryFile.write((char *)(&it->second.isExtern), sizeof(it->second.isExtern));
        binaryFile.write((char *)(&it->second.isLocal), sizeof(it->second.isLocal));
        binaryFile.write((char *)(&it->second.isSection), sizeof(it->second.isSection));
        binaryFile.write((char *)(&it->second.value), sizeof(it->second.value));

        stringLength = it->second.name.length();
        binaryFile.write((char *)(&stringLength), sizeof(stringLength));
        binaryFile.write(it->second.name.c_str(), it->second.name.length());

        stringLength = it->second.section.length();
        binaryFile.write((char *)(&stringLength), sizeof(stringLength));
        binaryFile.write(it->second.section.c_str(), it->second.section.length());
    }

    int numOfSections = sectionTable.size();
    binaryFile.write((char *)&numOfSections, sizeof(numOfSections));

    for (map<string, SectionTableEntry>::iterator it = sectionTable.begin(); it != sectionTable.end(); it++)
    {
        string key = it->first;
        unsigned int stringLength = key.length();
        binaryFile.write((char *)(&stringLength), sizeof(stringLength));
        binaryFile.write(key.c_str(), key.length());
    
        binaryFile.write((char *)(&it->second.rnumber), sizeof(it->second.rnumber));
        binaryFile.write((char *)(&it->second.needsPool), sizeof(it->second.needsPool));
        binaryFile.write((char *)(&it->second.poolSize), sizeof(it->second.poolSize));

        binaryFile.write((char *)(&it->second.size), sizeof(it->second.size));

        stringLength = it->second.name.length();
        binaryFile.write((char *)(&stringLength), sizeof(stringLength));
        binaryFile.write(it->second.name.c_str(), it->second.name.length());
        
        int numOfRelocations = relocationTable[key].size();

        binaryFile.write((char *)&numOfRelocations, sizeof(numOfRelocations));

        for (RelocationTableEntry rel : relocationTable[key])
        {

        binaryFile.write((char *)(&rel.addend), sizeof(rel.addend));
        binaryFile.write((char *)(&rel.isData), sizeof(rel.isData));
        binaryFile.write((char *)(&rel.offset), sizeof(rel.offset));

        unsigned int stringLength = rel.section.length();
        binaryFile.write((char *)(&stringLength), sizeof(stringLength));
        binaryFile.write(rel.section.c_str(), rel.section.length());

        stringLength = rel.symbol.length();
        binaryFile.write((char *)(&stringLength), sizeof(stringLength));
        binaryFile.write(rel.symbol.c_str(), rel.symbol.length());

    
    }
        int numOfChars = it->second.data.size();
        binaryFile.write((char *)&numOfChars, sizeof(numOfChars));

        int x = 0;
        for (char c : it->second.data)
        {
            x++;
            binaryFile.write((char *)&c, sizeof(c));
        }

        int numOfOffsets = it->second.offsets.size();
        binaryFile.write((char *)&numOfOffsets, sizeof(numOfOffsets));

        x = 0;
        for (long long o : it->second.offsets)
        {
            x++;
            binaryFile.write((char *)&o, sizeof(o));
        }
    }
        binaryFile.close();
    }
    else
    {
        string binary = outputPath;
        ofstream binaryFile(binary, ios::out | ios::binary);

        int numOfSec = sectionTable.size();

        map<string, SectionTableEntry>::iterator iter;
        iter = sectionTable.find("UND");
        if (iter != sectionTable.end())
        {
            numOfSec -= 1;
        }
        iter = sectionTable.find("ABS");
        if (iter != sectionTable.end())
        {
            numOfSec -= 1;
        }

        binaryFile.write((char *)&numOfSec, sizeof(numOfSec));
        for (map<string, SectionTableEntry>::iterator it = sectionTable.begin(); it != sectionTable.end(); it++)
        {
            if (it->first == "UND" || it->first == "ABS")
            {
                continue;
            }

            long long address = it->second.mappedAddress;
            binaryFile.write((char *)&address, sizeof(address));
            int numOfChars = it->second.data.size();
            binaryFile.write((char *)&numOfChars, sizeof(numOfChars));

            int x = 0;
            for (char c : it->second.data)
            {
                x++;
                binaryFile.write((char *)&c, sizeof(c));
            }
        }
    }
}

