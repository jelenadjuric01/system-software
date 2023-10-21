#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <map>
#include <string>
#include <iomanip>
#include <regex>


#include "../inc/assemblerHelp.hpp"
using namespace std;

long long Assembler::lowLimit=pow(-2,11);
long long Assembler::highLimit=pow(2,11)-1;
int Assembler::currentSymbolNumber = 0;
int Assembler::currentSectionNumber = 0;
string Assembler::filePath="";
string Assembler::outputPath="";
string Assembler::currentSection="";
unsigned long long Assembler::locationCounter=0;
map<string, Assembler::SymbolTableEntry> Assembler::symbolTable;
map<string,vector<Assembler::RelocationTableEntry>> Assembler::relocationTable;
map<string, Assembler::SectionTableEntry> Assembler::sectionTable;
vector<string> Assembler::equ;
map<string, Assembler::EquHelpTableEntry> Assembler::helpTable;
vector<string> Assembler::equPar;
Assembler::Operand Assembler::operand;
map<string,vector<Assembler::PoolOfLiterals>> Assembler::pool;
list<Assembler::EquHelpTableEntry> Assembler::equTable;

void Assembler::initialize(string path1, string outputPath1)
    {
    filePath=path1;
    outputPath=outputPath1;
    currentSymbolNumber = 0;
    currentSectionNumber = 0;
    sectionTable.clear();
    symbolTable.clear();
    relocationTable.clear();
    helpTable.clear();
    currentSection="";
    locationCounter=0;
    equ.clear();
    equPar.clear();
    operand.clear();
    pool.clear();
    equTable.clear();
    

    SectionTableEntry undfSection;
    undfSection.rnumber = currentSectionNumber++;
    undfSection.name = "UND";
    undfSection.size = 0;
    sectionTable["UND"] = undfSection;
    SymbolTableEntry undfSym;
    undfSym.rnumber = currentSymbolNumber++;
    undfSym.isLocal = true;
    undfSym.isSection=true;
    undfSym.isDefined=true;
    undfSym.isExtern=false;
    undfSym.name = "UND";
    undfSym.section = "UND";
    undfSym.value = 0;
    symbolTable["UND"] = undfSym;

   
    SectionTableEntry absSection;
    absSection.rnumber = -1;
    absSection.name = "ABS";
    absSection.size = 0;
    sectionTable["ABS"] = absSection;
    SymbolTableEntry absSym;
    absSym.rnumber = currentSymbolNumber++;
    absSym.isLocal = true;
    absSym.isDefined=true;
    absSym.isExtern=false;
    absSym.name = "ABS";
    absSym.isSection=true;
    absSym.section = "ABS";
    absSym.value = 0;
    symbolTable["ABS"] = absSym;

}


void Assembler::globalOrExternSymbol(string line, int lineNumber){
    regex pattern("\\.(global|extern)[ ][ \t]*([A-Za-z_][A-Za-z0-9_]*[ \t]*([ \t]*,[ \t]*[A-Za-z_][A-Za-z0-9_]*)*)");

        smatch matches;
        string symbols,keyword;
        if (regex_search(line, matches, pattern)) {
            symbols = matches[2].str();
            keyword=matches[1].str();
    }
    
    regex ws("\\s+");
    string cleaned=regex_replace(symbols,ws,"");
    regex delimiter(",");
    sregex_token_iterator it(cleaned.begin(),cleaned.end(),delimiter,-1);
    sregex_token_iterator end;
    while(it!=end){
        map<string,SymbolTableEntry>::iterator entry=symbolTable.find(*it);
        if(keyword=="global"){
            if(entry!=symbolTable.end()){
            if(entry->second.isExtern || entry->second.isSection)
            {   cout<<"Symbol "<<*it<<" is declared more times than one error on line: "<<lineNumber<<endl;
                exit(1);
                }
                else{
                    entry->second.isLocal=false;
                }
        }
        else{
            SymbolTableEntry newSymbol;
            newSymbol.isLocal=false;
            newSymbol.isSection=false;
            newSymbol.isDefined=false;
            newSymbol.isExtern=false;
            newSymbol.name=*it;
            newSymbol.rnumber=currentSymbolNumber++;
            newSymbol.section="UND";
            newSymbol.value=0;
            symbolTable[*it]=newSymbol;
        }
        }
        else{
            if(entry!=symbolTable.end()){
            if(entry->second.isSection || entry->second.isDefined)
            {   cout<<"Symbol "<<*it<<" is declared more times than one error on line: "<<lineNumber<<endl;
                exit(1);
                }
                else{
                    entry->second.isExtern=true;
                    entry->second.isLocal=false;
                }
            }
            else{
            SymbolTableEntry newSymbol;
            newSymbol.isLocal=false;
            newSymbol.isSection=false;
            newSymbol.isDefined=false;
            newSymbol.isExtern=true;
            newSymbol.name=*it;
            newSymbol.rnumber=currentSymbolNumber++;
            newSymbol.section="UND";
            newSymbol.value=0;
            symbolTable[*it]=newSymbol;
            }
        }
        it++;
        
    }

}


void Assembler::sectionFirstPass(string section, int lineNumber){
    map<string,SymbolTableEntry>::iterator sec=symbolTable.find(section);
    if(sec!=symbolTable.end()){
        cout<<"Section "<<section<<" is declared more times error on line:"<<lineNumber<<endl;
        exit(1);
    }
    if(currentSection!=""){
        sectionTable[currentSection].size=Assembler::locationCounter;
    }
    locationCounter=0;
    currentSection=section;

    SectionTableEntry newSection;
    newSection.name=section;
    newSection.rnumber=currentSectionNumber++;
    newSection.size=0;
    sectionTable[section]=newSection;

    SymbolTableEntry newSymbol;
    newSymbol.isDefined=true;
    newSymbol.isExtern=false;
    newSymbol.isLocal=true;
    newSymbol.isSection=true;
    newSymbol.name=section;
    newSymbol.rnumber=currentSymbolNumber++;
    newSymbol.section=section;
    newSymbol.value=0;

    symbolTable[section]=newSymbol;

}

long long Assembler::decimalFromLiteral(long long number){
    regex hexNumber("0[xX][0-9aAbBcCdDeEfF]+");
    smatch m;
    long long decimal;
    string s=to_string(number);
    if(regex_search(s,m,hexNumber)){
        stringstream ss;
        ss<<m.str(1).substr(2);
        ss>>hex>>decimal;
    }
    else{
        decimal=number;
    }
    return decimal;
}

void Assembler::skipFirstPass(long long literal, int lineNumber){
    if(currentSection==""){
        cout<<"Skip has to be inside of a section! Error on line:"<<lineNumber<<endl;
        exit(1);
    }
    long long value=decimalFromLiteral(literal);
    locationCounter+=value;

}

void Assembler::wordFirstPass(string symbol,int lineNumber){
    if(currentSection==""){
        cout<<"Word directive has to be in section, error on line:"<<lineNumber<<endl;
        exit(1);
    }
    if(symbol!=""){
         map<string,SymbolTableEntry>::iterator sec=symbolTable.find(symbol);
        if(sec==symbolTable.end()){
            SymbolTableEntry newSymbol;
            newSymbol.isLocal=true;
            newSymbol.isSection=false;
            newSymbol.isDefined=false;
            newSymbol.isExtern=false;
            newSymbol.name=symbol;
            newSymbol.rnumber=currentSymbolNumber++;
            newSymbol.section="UND";
            newSymbol.value=0;
            symbolTable[symbol]=newSymbol;
            }
       
        }
    locationCounter+=4;
}

void Assembler::asciiFirstPass(string arg,int lineNumber){
    if(currentSection==""){
        cout<<"Ascii directive has to be in section, error on line:"<<lineNumber<<endl;
        exit(1);
    }
    locationCounter+=arg.length()-2;
}
void Assembler::equFirstPass(string symbol, int lineNumber){
   
   
        map<string,SymbolTableEntry>::iterator entry=symbolTable.find(symbol);
       
            if(entry!=symbolTable.end()){
            if(entry->second.isExtern || entry->second.isSection || entry->second.isDefined)
            {   cout<<"Equ directive cant define symbol "<<symbol<<" because it is either declared external,is a section or is already defined,error on line: "<<lineNumber<<endl;
                exit(1);
                }
                else{
                    entry->second.section="ABS";

                }
        }
        else{
            SymbolTableEntry newSymbol;
            newSymbol.isLocal=true;
            newSymbol.isSection=false;
            newSymbol.isDefined=false;
            newSymbol.isExtern=false;
            newSymbol.name=symbol;
            newSymbol.rnumber=currentSymbolNumber++;
            newSymbol.section="ABS";
            newSymbol.value=0;
            symbolTable[symbol]=newSymbol;
        
        }
        EquHelpTableEntry newEntry;
        newEntry.symbol=symbol;
        newEntry.equals.assign(equPar.begin(),equPar.end());
        helpTable[symbol]=newEntry;
        equTable.push_back(newEntry);
        equPar.clear();


}
void Assembler::labelFirstPass(string label,int lineNumber){
    label.erase(label.size()-1);
    if(currentSection==""){
        cout<<"Label "<<label<<" defined on line "<<lineNumber<<" has to be inside of section!"<<endl;
        exit(1);
    }
    map<string,SymbolTableEntry>::iterator entry=symbolTable.find(label);
    if(entry!=symbolTable.end()){
        if(entry->second.isDefined){
            cout<<"Label "<<label<<" defined on line "<<lineNumber<<" is aldready defined"<<endl;
            exit(1);
        }
        if(entry->second.isExtern){
            cout<<"Label "<<label<<" defined on line "<<lineNumber<<" is declared extern"<<endl;
            exit(1);
        }
        if(entry->second.isSection){
            cout<<"Label "<<label<<" defined on line "<<lineNumber<<" is a section"<<endl;
            exit(1);
        }
        entry->second.isDefined=true;
        entry->second.value=locationCounter;
        entry->second.section=currentSection;
    }
    else{
        SymbolTableEntry newSymbol;
            newSymbol.isLocal=true;
            newSymbol.isSection=false;
            newSymbol.isDefined=true;
            newSymbol.isExtern=false;
            newSymbol.name=label;
            newSymbol.rnumber=currentSymbolNumber++;
            newSymbol.section=currentSection;
            newSymbol.value=locationCounter;
            symbolTable[label]=newSymbol;
    }
}
void Assembler::instructionFirstPass(int lineNumber){
    if(currentSection==""){
        cout<<"Instruction has to be inside of a section! Error on line:"<<lineNumber<<endl;
        exit(1);
    }
    
    locationCounter+=4;

}
void Assembler::endFirstPass(){
    bool noError=true;
    if(currentSection!=""){
        sectionTable[currentSection].size=Assembler::locationCounter;
    }

    for(map<string,Assembler::SymbolTableEntry>::iterator it=symbolTable.begin();it!=symbolTable.end();it++){
        SymbolTableEntry s=it->second;
        if(s.section=="ABS") continue;
        if(s.isLocal && s.isExtern){
            cout<<"Symbol "<<s.name<<" can not be extern and local at the same time!"<<endl;
            noError=false;
        }
        if(s.isLocal && !s.isDefined){
            cout<<"Local symbol "<<s.name<<" must be defined!"<<endl;
            noError=false;
        }
        if(!s.isExtern && !s.isDefined){
            cout<<"Symbol "<<s.name<<" must be defined!"<<endl;
            noError=false;
        }
        
    }
    if(!noError) exit(1);
    if(!helpTable.empty()){
        equCalculateValue();
        for(map<string,EquHelpTableEntry>::iterator it=helpTable.begin();it!=helpTable.end();it++){
            equ.clear();
            it->second.type=equCheck(it->second);
            map<string,SymbolTableEntry>::iterator iter=symbolTable.find(it->second.symbol);
          

        }
    }
    for(map<string,SectionTableEntry>::iterator it=sectionTable.begin();it!=sectionTable.end();it++){
        map<string,vector<PoolOfLiterals>>::iterator iter=pool.find(it->second.name);
        if(iter!=pool.end()){
            if(iter->second.size()==0){
                pool.erase(iter);
                it->second.needsPool=false;
                if(it->second.size>pow(2,32)){
                    cout<<"Section "<<it->second.name<<" needs to be smaller than 2^32b."<<endl;
                    exit(1);
                }
            }
            else{
                if(it->second.size>pow(2,12)){
                    cout<<"Section "<<it->second.name<<" needs to be smaller than 2^12b because it needs pool of literals."<<endl;
                    exit(1);
                }
            }
            long long curr=it->second.size;
            for(size_t i=0;i<iter->second.size();i++){
                iter->second[i].adress=curr;
                curr+=4;
            }
            it->second.poolSize=iter->second.size()*4;
        }
        else if(it->second.size>pow(2,32)){
                    cout<<"Section "<<it->second.name<<" needs to be smaller than 2^32b."<<endl;
                    exit(1);
        }
    }
    locationCounter=0;
}
void Assembler::sectionSecondPass(string section){    
    locationCounter=0;
    currentSection=section;
}
void Assembler::wordSecondPass(string symbol,long long literal,int lineNumber){
    
    if(symbol!=""){
         map<string,SymbolTableEntry>::iterator sec=symbolTable.find(symbol);
        if(sec!=symbolTable.end()){
            SymbolTableEntry sym=sec->second;
            if(sym.section=="ABS"){
                if(helpTable[sym.name].type=="ABS"){
                    sectionTable[currentSection].offsets.push_back(locationCounter);
                    sectionTable[currentSection].data.push_back(0xff&sym.value);
                    sectionTable[currentSection].data.push_back(0xff&(sym.value>>8));
                    sectionTable[currentSection].data.push_back(0xff&(sym.value>>16));
                    sectionTable[currentSection].data.push_back(0xff&(sym.value>>24));

                }
                else{
                    sectionTable[currentSection].offsets.push_back(locationCounter);
                    sectionTable[currentSection].data.push_back(0xff&0);
                    sectionTable[currentSection].data.push_back(0xff&0);
                    sectionTable[currentSection].data.push_back(0xff&0);
                    sectionTable[currentSection].data.push_back(0xff&0);

                    RelocationTableEntry rel;
                    rel.isData=true;
                    rel.section=currentSection;
                    rel.symbol=(sym.isLocal?helpTable[sym.name].type:sym.name);
                    rel.offset=locationCounter;
                    rel.addend=(sym.isLocal?sym.value:0);//PROVERI
                    relocationTable[currentSection].push_back(rel);
                }
            }
            else{
                
                    sectionTable[currentSection].offsets.push_back(locationCounter);
                    sectionTable[currentSection].data.push_back(0xff&0);
                    sectionTable[currentSection].data.push_back(0xff&0);
                    sectionTable[currentSection].data.push_back(0xff&0);
                    sectionTable[currentSection].data.push_back(0xff&0);
                    RelocationTableEntry rel;
                    rel.isData=true;
                    rel.section=currentSection;
                    rel.offset=locationCounter;
                    rel.addend=(sym.isLocal?sym.value:0);
                    rel.symbol=(sym.isLocal?sym.section:sym.name);
                    relocationTable[currentSection].push_back(rel);
                
            }
        }
    }
    else{
        char value=decimalFromLiteral(literal);
        sectionTable[currentSection].offsets.push_back(locationCounter);
        sectionTable[currentSection].data.push_back(0xff&value);
        sectionTable[currentSection].data.push_back(0xff&(value>>8));
        sectionTable[currentSection].data.push_back(0xff&(value>>16));
        sectionTable[currentSection].data.push_back(0xff&(value>>24));

    }
    locationCounter+=4;

}
void Assembler::addToEquFirstPass(string symbol,string oper,long long number,bool operandValid){
   
    if(symbol!="" && oper!=""){
        map<string,SymbolTableEntry>::iterator sec=symbolTable.find(symbol);
        if(sec!=symbolTable.end()){
            equPar.insert(equPar.begin(),oper);
            equPar.insert(equPar.begin(),sec->second.name);


        }
        else{
            SymbolTableEntry newSymbol;
            newSymbol.isLocal=true;
            newSymbol.isSection=false;
            newSymbol.isDefined=false;
            newSymbol.isExtern=false;
            newSymbol.name=symbol;
            newSymbol.rnumber=currentSymbolNumber++;
            newSymbol.section="UND";
            newSymbol.value=0;
            symbolTable[symbol]=newSymbol;
            equPar.insert(equPar.begin(),oper);
            equPar.insert(equPar.begin(),symbol);
        }
        
    }
    else if(operandValid && oper!=""){
        equPar.insert(equPar.begin(),oper);
        equPar.insert(equPar.begin(),to_string(number));
       
    }
    else if(operandValid){
        equPar.insert(equPar.begin(),to_string(number));
    }
    else if(symbol!=""){
        map<string,SymbolTableEntry>::iterator sec=symbolTable.find(symbol);
        if(sec!=symbolTable.end()){
            equPar.insert(equPar.begin(),sec->second.name);


        }
        else{
            SymbolTableEntry newSymbol;
            newSymbol.isLocal=true;
            newSymbol.isSection=false;
            newSymbol.isDefined=false;
            newSymbol.isExtern=false;
            newSymbol.name=symbol;
            newSymbol.rnumber=currentSymbolNumber++;
            newSymbol.section="UND";
            newSymbol.value=0;
            symbolTable[symbol]=newSymbol;
            equPar.insert(equPar.begin(),symbol);
        }
    }
}
string Assembler::equCheck(EquHelpTableEntry entry){
    for(string elem:entry.equals){
        if(elem=="+" || elem=="-"){
            equ.push_back(elem);
        }
        else if(isdigit(elem[0]) || elem[0]=='-'){
            equ.push_back("0");
        }
        else{
            map<string,SymbolTableEntry>::iterator bogdan=symbolTable.find(elem);
            if(bogdan==symbolTable.end()){
                cout<<"Equ direktiva sadrzi nepostojeci simbol"<<endl;
                exit(1);
            }
            SymbolTableEntry symbol=bogdan->second;
            if(symbol.section=="ABS" && symbol.isDefined){
                equ.push_back("0");
            }
            else if(symbol.isExtern){
                equ.push_back("UND("+symbol.name+")");
            }
            else{
                equ.push_back(symbol.section);
            }
        }
    }
    if(!equ.empty())
    {    map<string,long long> counter;
        map<string,vector<string>> symbols;
        for(size_t i=0;i<equ.size();i++){
            if(equ[i]=="0"){
                    continue;
                }
            else if(equ[i]=="+"){
                    string ident=equ[i+1];
                    if(ident!="0"){
                        map<string,long long>::iterator j=counter.find(ident);
                        if(j!=counter.end()){
                            j->second++;
                        }
                        else{
                            counter[ident]=1;
                        }
                        i++;
                    }
                }
            else if(equ[i]=="-"){
                string ident=equ[i+1];
                    if(ident!="0"){
                        map<string,long long>::iterator j=counter.find(ident);
                        if(j!=counter.end()){
                            j->second--;
                        }
                        else{
                            counter[ident]=-1;
                        }
                        i++;
                    }
            }
            else{
                counter[equ[i]]=1;
                symbols[equ[i]].push_back(equPar[i]);
            }
        }
        if(counter.empty()) return "ABS";
        int c=0;
        for(map<string,long long>::iterator it=counter.begin();it!=counter.end();it++){
            if(it->second!=0){
                c++;
            }
        }
        if(c>1){
            cout<<"Equ direktiva koristi simbole koji su nekorektno relokatibilni"<<endl;
            exit(1);
        }
        if(c==0){
            return "ABS";
        }
        if(c==1){
            string s=counter.begin()->first;
            if(s[3]=='('){
                entry.typeSymbol=s.substr(4,s.size()-5);
                return s.substr(4,s.size()-5);
            }
            else{
                return s;
            }
        }
        
    }
    return "ABS";
}
void Assembler::equCalculateValue(){
    int n=pow(equTable.size()*2,2);
    while(equTable.size()>0 && n>0){
        bool error=false;
    vector<string> e=equTable.front().equals;
    long long value=0;
    for(size_t i=0;i<e.size();i++){
        if(isdigit(e[i][0]) || (e[i][0]=='-' && e[i].size()>1)){
            value+=stoi(e[i]);
        }
        else if(e[i]=="+"){
            string next=e[i+1];
            if(isdigit(next[0]) || next[0]=='-'){
                value+=stoi(next);
            }
            else{
                if(symbolTable[next].section=="ABS" && symbolTable[next].isDefined==false){
                EquHelpTableEntry eq=equTable.front();
                equTable.erase(equTable.begin());
                equTable.push_back(eq);
                error=true;
                break;
                
            }
                value+=symbolTable[next].value;
            }
            i++;
        }
        else if(e[i]=="-"){
            string next=e[i+1];
            if(isdigit(next[0]) || next[0]=='-'){
                value-=stoi(next);
            }
            else{
                 if(symbolTable[next].section=="ABS" && symbolTable[next].isDefined==false){
                EquHelpTableEntry eq=equTable.front();
                equTable.erase(equTable.begin());
                equTable.push_back(eq);
                error=true;
                break;
            }
                value-=symbolTable[next].value;
            }
            i++;
        }
        else{
            if(symbolTable[e[i]].section=="ABS" && symbolTable[e[i]].isDefined==false){
                EquHelpTableEntry eq=equTable.front();
                equTable.erase(equTable.begin());
                equTable.push_back(eq);
                error=true;
                break;
            }
            value+=symbolTable[e[i]].value;

        }
    }
    if(!error){
        symbolTable[equTable.front().symbol].isDefined=true;
        symbolTable[equTable.front().symbol].value=value;
        equTable.erase(equTable.begin());
    }
    n--;
    }
    if(n==0){
        cout<<"There are unresolved equ symbols"<<endl;
        exit(1);
    }
}


void Assembler::skipSecondPass(long long literal){
    long long value=decimalFromLiteral(literal);
    sectionTable[currentSection].offsets.push_back(locationCounter);
    for(int i=0;i<value;i++){
        sectionTable[currentSection].data.push_back(0);

    }
    locationCounter+=value;
}
void Assembler::asciiSecondPass(string s){
    string str=s.substr(1,s.size()-2);
    sectionTable[currentSection].offsets.push_back(locationCounter);

    for(int i=0;i<str.size();i++){
        sectionTable[currentSection].data.push_back(0xff&(int)str[i]);

    }
    locationCounter+=str.size();
}
void Assembler::equSecondPass(string symbol){
    map<string,EquHelpTableEntry>::iterator it=helpTable.find(symbol);
    EquHelpTableEntry eq=it->second;
    if(eq.type!="ABS" && !symbolTable[eq.symbol].isLocal){
        SymbolTableEntry sym=symbolTable[eq.symbol];
        RelocationTableEntry rel;
        rel.isData=false;
        rel.section="ABS";
        rel.symbol=helpTable[sym.name].type;
        rel.offset=sym.rnumber;
        rel.addend=sym.value;
        relocationTable["ABS"].push_back(rel);
    }
}
void Assembler::addDir(bool dir){
    operand.isDir=dir;
}
void Assembler::addJmp(bool dir){
    operand.isJump=dir;
}
void Assembler::addLiteral(long long dir){
    operand.literal=decimalFromLiteral(dir);
}
void Assembler::addReg(string r1){
    if(r1[1]=='s') r1="%r14";
    else if(r1[1]=='p') r1="%r15";
    operand.reg=stoi(r1.substr(2));
}
void Assembler::addSymbol(string dir){
    operand.symbol=dir;
}
void Assembler::iretFirstPass(){
    locationCounter+=8;
}
void Assembler::parseInstructionFirstPass(int inst, int lineNumber){
    if(currentSection==""){
        cout<<"Instruction must be inside of section, error on line:"<<lineNumber<<endl;
        exit(1);
    }
    switch(inst){
        case 0://jmp,call,beq,bne,bgt
            if(operand.isJump==false){
                cout<<"Error on line:"<<lineNumber<<endl;
                exit(1);
            }
            if(operand.reg>0){
                cout<<"Error on line:"<<lineNumber<<endl;
                exit(1);
            }
            if(operand.symbol==""){
                long long literal=operand.literal;
                if(literal<lowLimit || literal>highLimit){
                    map<string,vector<PoolOfLiterals>>::iterator it=pool.find(currentSection);
                    if(it!=pool.end()){
                        bool exists=false;
                        for(auto i:it->second){
                            if(i.isConst && i.value==literal){
                                exists=true;
                                break;
                            }
                        }
                        if(!exists){
                            PoolOfLiterals entry;
                            entry.isConst=true;
                            entry.value=literal;
                            it->second.push_back(entry);
                        }
                    }
                    else{
                        vector<PoolOfLiterals> v;
                        PoolOfLiterals entry;
                        entry.isConst=true;
                        entry.value=literal;
                        v.push_back(entry);
                        pool[currentSection]=v;
                    }
                    sectionTable[currentSection].needsPool=true;

                }
            }
            else{
                map<string,SymbolTableEntry>::iterator iter=symbolTable.find(operand.symbol);
                if(iter==symbolTable.end()){
                    string symbol=operand.symbol;
                SymbolTableEntry newSymbol;
                newSymbol.isLocal=true;
                newSymbol.isSection=false;
                newSymbol.isDefined=false;
                newSymbol.isExtern=false;
                newSymbol.name=symbol;
                newSymbol.rnumber=currentSymbolNumber++;
                newSymbol.section="UND";
                newSymbol.value=0;
                symbolTable[symbol]=newSymbol;
                }
                map<string,vector<PoolOfLiterals>>::iterator it=pool.find(currentSection);
                    if(it!=pool.end()){
                        bool exists=false;
                        for(auto i:it->second){
                            if(!i.isConst && i.symbol==operand.symbol){
                                exists=true;
                                break;
                            }
                        }
                        if(!exists){
                            PoolOfLiterals entry;
                            entry.isConst=false;
                            entry.symbol=operand.symbol;
                            it->second.push_back(entry);
                        }
                    }
                    else{
                        vector<PoolOfLiterals> v;
                        PoolOfLiterals entry;
                        entry.isConst=false;
                        entry.symbol=operand.symbol;
                        v.push_back(entry);
                        pool[currentSection]=v;
                    }

                   sectionTable[currentSection].needsPool=true;


            }
        break;
        case 1://ld
            if(operand.reg!=-1){
                long long literal=operand.literal;
                if(literal<lowLimit || literal>highLimit){
                    cout<<"Literal must not be longer than 12b, error on line:"<<lineNumber<<endl;
                    exit(1);
                }
            }
            else{
                if(operand.symbol==""){
                long long literal=operand.literal;
                if(literal<lowLimit || literal>highLimit){
                map<string,vector<PoolOfLiterals>>::iterator it=pool.find(currentSection);
                    if(it!=pool.end()){
                        bool exists=false;
                        for(auto i:it->second){
                            if(i.isConst && i.value==literal){
                                exists=true;
                                break;
                            }
                        }
                        if(!exists){
                            PoolOfLiterals entry;
                            entry.isConst=true;
                            entry.value=literal;
                            it->second.push_back(entry);
                        }
                    }
                    else{
                        vector<PoolOfLiterals> v;
                        PoolOfLiterals entry;
                        entry.isConst=true;
                        entry.value=literal;
                        v.push_back(entry);
                        pool[currentSection]=v;
                    }
                    sectionTable[currentSection].needsPool=true;
                    if(operand.isDir) locationCounter+=4;
                }
            }
            else{
                map<string,SymbolTableEntry>::iterator iter=symbolTable.find(operand.symbol);
                if(iter==symbolTable.end()){
                    string symbol=operand.symbol;
                SymbolTableEntry newSymbol;
                newSymbol.isLocal=true;
                newSymbol.isSection=false;
                newSymbol.isDefined=false;
                newSymbol.isExtern=false;
                newSymbol.name=symbol;
                newSymbol.rnumber=currentSymbolNumber++;
                newSymbol.section="UND";
                newSymbol.value=0;
                symbolTable[symbol]=newSymbol;
                }
                map<string,vector<PoolOfLiterals>>::iterator it=pool.find(currentSection);
                    if(it!=pool.end()){
                        bool exists=false;
                        for(auto i:it->second){
                            if(!i.isConst && i.symbol==operand.symbol){
                                exists=true;
                                break;
                            }
                        }
                        if(!exists){
                            PoolOfLiterals entry;
                            entry.isConst=false;
                            entry.symbol=operand.symbol;
                            it->second.push_back(entry);
                        }
                    }
                    else{
                        vector<PoolOfLiterals> v;
                        PoolOfLiterals entry;
                        entry.isConst=false;
                        entry.symbol=operand.symbol;
                        v.push_back(entry);
                        pool[currentSection]=v;
                    }

                   sectionTable[currentSection].needsPool=true;
                    if(operand.isDir) locationCounter+=4;

            }

            }
        break;
        case 2://st
        if(operand.reg!=-1){
               
                long long literal=operand.literal;
                if(literal<lowLimit || literal>highLimit){
                    cout<<"Literal must not be longer than 12b, error on line:"<<lineNumber<<endl;
                    exit(1);
                }
                

            }
            else if(operand.isDir==false){
                cout<<"Store instruction error on line:"<<lineNumber<<endl;
            }
            else{
                if(operand.symbol==""){
                
                long long literal=operand.literal;
                if(literal<lowLimit || literal>highLimit){
                map<string,vector<PoolOfLiterals>>::iterator it=pool.find(currentSection);
                    if(it!=pool.end()){
                        bool exists=false;
                        for(auto i:it->second){
                            if(i.isConst && i.value==literal){
                                exists=true;
                                break;
                            }
                        }
                        if(!exists){
                            PoolOfLiterals entry;
                            entry.isConst=true;
                            entry.value=literal;
                            it->second.push_back(entry);
                        }
                    }
                    else{
                        vector<PoolOfLiterals> v;
                        PoolOfLiterals entry;
                        entry.isConst=true;
                        entry.value=literal;
                        v.push_back(entry);
                        pool[currentSection]=v;
                    }
                    sectionTable[currentSection].needsPool=true;
                }
            }
            else{
                map<string,SymbolTableEntry>::iterator iter=symbolTable.find(operand.symbol);
                if(iter==symbolTable.end()){
                    string symbol=operand.symbol;
                SymbolTableEntry newSymbol;
                newSymbol.isLocal=true;
                newSymbol.isSection=false;
                newSymbol.isDefined=false;
                newSymbol.isExtern=false;
                newSymbol.name=symbol;
                newSymbol.rnumber=currentSymbolNumber++;
                newSymbol.section="UND";
                newSymbol.value=0;
                symbolTable[symbol]=newSymbol;
                }
                map<string,vector<PoolOfLiterals>>::iterator it=pool.find(currentSection);
                    if(it!=pool.end()){
                        bool exists=false;
                        for(auto i:it->second){
                            if(!i.isConst && i.symbol==operand.symbol){
                                exists=true;
                                break;
                            }
                        }
                        if(!exists){
                            PoolOfLiterals entry;
                            entry.isConst=false;
                            entry.symbol=operand.symbol;
                            it->second.push_back(entry);
                        }
                    }
                    else{
                        vector<PoolOfLiterals> v;
                        PoolOfLiterals entry;
                        entry.isConst=false;
                        entry.symbol=operand.symbol;
                        v.push_back(entry);
                        pool[currentSection]=v;
                    }

                   sectionTable[currentSection].needsPool=true;
            }

    }
    break;
            }
            

    operand.clear();
}


void Assembler::endSecondPass(int lineNumber){

    
    for(map<string,vector<PoolOfLiterals>>::iterator it=pool.begin();it!=pool.end();it++){
        long long lc=sectionTable[it->first].size;
        for(PoolOfLiterals& elem:it->second){
            if(!elem.isConst){
                if(symbolTable[elem.symbol].section=="ABS" && helpTable[elem.symbol].type=="ABS"){
                    sectionTable[it->first].offsets.push_back(lc);
                    sectionTable[it->first].data.push_back(0xff&symbolTable[elem.symbol].value);
                    sectionTable[it->first].data.push_back(0xff&(symbolTable[elem.symbol].value>>8));
                    sectionTable[it->first].data.push_back(0xff&(symbolTable[elem.symbol].value>>16));
                    sectionTable[it->first].data.push_back(0xff&(symbolTable[elem.symbol].value>>24));
                }
                else{
                    sectionTable[it->first].offsets.push_back(lc);
                    sectionTable[it->first].data.push_back(0x00);
                    sectionTable[it->first].data.push_back(0x00);
                    sectionTable[it->first].data.push_back(0x00);
                    sectionTable[it->first].data.push_back(0x00);
                    RelocationTableEntry rel;
                    rel.isData=true;
                    rel.section=it->first;
                    if(symbolTable[elem.symbol].section=="ABS"){
                        rel.symbol=(symbolTable[elem.symbol].isLocal?helpTable[elem.symbol].type:elem.symbol);

                    }
                    else
{                    rel.symbol=(symbolTable[elem.symbol].isLocal?it->first:elem.symbol);
}                    rel.offset=lc;
                    rel.addend=(symbolTable[elem.symbol].isLocal?symbolTable[elem.symbol].value:0);
                    relocationTable[currentSection].push_back(rel);
                }
            }
            else{
                sectionTable[it->first].offsets.push_back(lc);
                sectionTable[it->first].data.push_back(0xff&elem.value);
                sectionTable[it->first].data.push_back(0xff&(elem.value>>8));
                sectionTable[it->first].data.push_back(0xff&(elem.value>>16));
                sectionTable[it->first].data.push_back(0xff&(elem.value>>24));
            }
            lc+=4;
        }
    }
}
void Assembler::instructionSecondPassNoPool(string instr,string r1,string r2, int lineNumber){
    int code=0;
    if(r1[1]=='s') r1="%r14";
    else if(r1[1]=='p') r1="%r15";
    if(r2[1]=='s') r2="%r14";
    else if(r2[1]=='p') r2="%r15";
    if(instr=="halt"){
        sectionTable[currentSection].offsets.push_back(locationCounter);
        sectionTable[currentSection].data.push_back(0x00);
         sectionTable[currentSection].data.push_back(0x00);
        sectionTable[currentSection].data.push_back(0x00);
        sectionTable[currentSection].data.push_back(0x00);

        locationCounter+=4;
    }
    else if(instr=="int"){
        sectionTable[currentSection].offsets.push_back(locationCounter);
        sectionTable[currentSection].data.push_back(0x00);
         sectionTable[currentSection].data.push_back(0x00);
        sectionTable[currentSection].data.push_back(0x00);
        sectionTable[currentSection].data.push_back(0x10);
    
        locationCounter+=4;   
    }
    else if(instr=="iret"){
        code=0b10010001111011100000000000001000;
        sectionTable[currentSection].offsets.push_back(locationCounter);
        sectionTable[currentSection].data.push_back(code &0xff);
         sectionTable[currentSection].data.push_back(0xff&(code>>8));
        sectionTable[currentSection].data.push_back(0xff&(code>>16));
        sectionTable[currentSection].data.push_back(0xff&(code>>24));
        code=0b10010110000011100000111111111100;
        sectionTable[currentSection].data.push_back(code &0xff);
         sectionTable[currentSection].data.push_back(0xff&(code>>8));
        sectionTable[currentSection].data.push_back(0xff&(code>>16));
        sectionTable[currentSection].data.push_back(0xff&(code>>24));
        code=0b00111000111000000000111111111000;
        sectionTable[currentSection].data.push_back(code &0xff);
         sectionTable[currentSection].data.push_back(0xff&(code>>8));
        sectionTable[currentSection].data.push_back(0xff&(code>>16));
        sectionTable[currentSection].data.push_back(0xff&(code>>24));
    
        locationCounter+=12;   
    }
    else if(instr=="ret"){
        code=0b10010011111111100000000000000100;
        sectionTable[currentSection].offsets.push_back(locationCounter);
        sectionTable[currentSection].data.push_back(code &0xff);
         sectionTable[currentSection].data.push_back(0xff&(code>>8));
        sectionTable[currentSection].data.push_back(0xff&(code>>16));
        sectionTable[currentSection].data.push_back(0xff&(code>>24));
        locationCounter+=4;
    }
    else if(instr=="push "){
        int r=stoi(r1.substr(2))<<12;
        code=0b10000001111000000000111111111100 | r;
        sectionTable[currentSection].offsets.push_back(locationCounter);
        sectionTable[currentSection].data.push_back(code &0xff);
         sectionTable[currentSection].data.push_back(0xff&(code>>8));
        sectionTable[currentSection].data.push_back(0xff&(code>>16));
        sectionTable[currentSection].data.push_back(0xff&(code>>24));
        locationCounter+=4;
    }
    else if(instr=="pop "){
        int r=stoi(r1.substr(2))<<20;
        code=0b10010011000011100000000000000100 | r;
        sectionTable[currentSection].offsets.push_back(locationCounter);
        sectionTable[currentSection].data.push_back(code &0xff);
         sectionTable[currentSection].data.push_back(0xff&(code>>8));
        sectionTable[currentSection].data.push_back(0xff&(code>>16));
        sectionTable[currentSection].data.push_back(0xff&(code>>24));
        locationCounter+=4;
    }
    else if(instr=="xchg "){
        int rs=stoi(r1.substr(2))<<12;
        int rd=stoi(r2.substr(2))<<16;
        code=0b01000000000000000000000000000000 | rs | rd;
        sectionTable[currentSection].offsets.push_back(locationCounter);
        sectionTable[currentSection].data.push_back(code &0xff);
         sectionTable[currentSection].data.push_back(0xff&(code>>8));
        sectionTable[currentSection].data.push_back(0xff&(code>>16));
        sectionTable[currentSection].data.push_back(0xff&(code>>24));
        locationCounter+=4;

    }
    else if(instr=="add "){
        int rs=stoi(r1.substr(2))<<12;
        int rd=stoi(r2.substr(2))<<16;
        int r=rd<<4;
        code=0b01010000000000000000000000000000| rs | rd | r;
        sectionTable[currentSection].offsets.push_back(locationCounter);
        sectionTable[currentSection].data.push_back(code &0xff);
         sectionTable[currentSection].data.push_back(0xff&(code>>8));
        sectionTable[currentSection].data.push_back(0xff&(code>>16));
        sectionTable[currentSection].data.push_back(0xff&(code>>24));
        locationCounter+=4;
    }
    else if(instr=="sub "){
        int rs=stoi(r1.substr(2))<<12;
        int rd=stoi(r2.substr(2))<<16;
        int r=rd<<4;
        code=0b01010001000000000000000000000000| rs | rd | r;
        sectionTable[currentSection].offsets.push_back(locationCounter);
        sectionTable[currentSection].data.push_back(code &0xff);
         sectionTable[currentSection].data.push_back(0xff&(code>>8));
        sectionTable[currentSection].data.push_back(0xff&(code>>16));
        sectionTable[currentSection].data.push_back(0xff&(code>>24));
        locationCounter+=4;
    }
    else if(instr=="mul "){
        int rs=stoi(r1.substr(2))<<12;
        int rd=stoi(r2.substr(2))<<16;
        int r=rd<<4;
        code=0b01010010000000000000000000000000| rs | rd | r;
        sectionTable[currentSection].offsets.push_back(locationCounter);
        sectionTable[currentSection].data.push_back(code &0xff);
         sectionTable[currentSection].data.push_back(0xff&(code>>8));
        sectionTable[currentSection].data.push_back(0xff&(code>>16));
        sectionTable[currentSection].data.push_back(0xff&(code>>24));
        locationCounter+=4;
    }
    else if(instr=="div "){
        int rs=stoi(r1.substr(2))<<12;
        int rd=stoi(r2.substr(2))<<16;
        int r=rd<<4;
        code=0b01010011000000000000000000000000| rs | rd | r;
        sectionTable[currentSection].offsets.push_back(locationCounter);
        sectionTable[currentSection].data.push_back(code &0xff);
         sectionTable[currentSection].data.push_back(0xff&(code>>8));
        sectionTable[currentSection].data.push_back(0xff&(code>>16));
        sectionTable[currentSection].data.push_back(0xff&(code>>24));
        locationCounter+=4;
    }
    else if(instr=="not "){
        int rs=stoi(r1.substr(2))<<16;
        int r=rs<<4;
        code=0b01100000000000000000000000000000| rs | r;
        sectionTable[currentSection].offsets.push_back(locationCounter);
        sectionTable[currentSection].data.push_back(code &0xff);
         sectionTable[currentSection].data.push_back(0xff&(code>>8));
        sectionTable[currentSection].data.push_back(0xff&(code>>16));
        sectionTable[currentSection].data.push_back(0xff&(code>>24));
        locationCounter+=4;
    }
    else if(instr=="and "){
        int rs=stoi(r1.substr(2))<<12;
        int rd=stoi(r2.substr(2))<<16;
        int r=rd<<4;
        code=0b01100001000000000000000000000000| rs | rd | r;
        sectionTable[currentSection].offsets.push_back(locationCounter);
        sectionTable[currentSection].data.push_back(code &0xff);
         sectionTable[currentSection].data.push_back(0xff&(code>>8));
        sectionTable[currentSection].data.push_back(0xff&(code>>16));
        sectionTable[currentSection].data.push_back(0xff&(code>>24));
        locationCounter+=4;
    }
    else if(instr=="or "){
        int rs=stoi(r1.substr(2))<<12;
        int rd=stoi(r2.substr(2))<<16;
        int r=rd<<4;
        code=0b01100010000000000000000000000000| rs | rd | r;
        sectionTable[currentSection].offsets.push_back(locationCounter);
        sectionTable[currentSection].data.push_back(code &0xff);
         sectionTable[currentSection].data.push_back(0xff&(code>>8));
        sectionTable[currentSection].data.push_back(0xff&(code>>16));
        sectionTable[currentSection].data.push_back(0xff&(code>>24));
        locationCounter+=4;
    }
    else if(instr=="xor "){
        int rs=stoi(r1.substr(2))<<12;
        int rd=stoi(r2.substr(2))<<16;
        int r=rd<<4;
        code=0b01100011000000000000000000000000| rs | rd | r;
        sectionTable[currentSection].offsets.push_back(locationCounter);
        sectionTable[currentSection].data.push_back(code &0xff);
         sectionTable[currentSection].data.push_back(0xff&(code>>8));
        sectionTable[currentSection].data.push_back(0xff&(code>>16));
        sectionTable[currentSection].data.push_back(0xff&(code>>24));
        locationCounter+=4;
    }
    else if(instr=="shl "){
        int rs=stoi(r1.substr(2))<<12;
        int rd=stoi(r2.substr(2))<<16;
        int r=rd<<4;
        code=0b01110000000000000000000000000000| rs | rd | r;
        sectionTable[currentSection].offsets.push_back(locationCounter);
        sectionTable[currentSection].data.push_back(code &0xff);
         sectionTable[currentSection].data.push_back(0xff&(code>>8));
        sectionTable[currentSection].data.push_back(0xff&(code>>16));
        sectionTable[currentSection].data.push_back(0xff&(code>>24));
        locationCounter+=4;
    }
    else if(instr=="shr "){
        int rs=stoi(r1.substr(2))<<12;
        int rd=stoi(r2.substr(2))<<16;
        int r=rd<<4;
        code=0b01110001000000000000000000000000| rs | rd | r;
        sectionTable[currentSection].offsets.push_back(locationCounter);
        sectionTable[currentSection].data.push_back(code &0xff);
         sectionTable[currentSection].data.push_back(0xff&(code>>8));
        sectionTable[currentSection].data.push_back(0xff&(code>>16));
        sectionTable[currentSection].data.push_back(0xff&(code>>24));
        locationCounter+=4;
    }
    else if(instr=="csrrd"){
        int rc=(r1=="%status"? 0:r1=="%handler"? 1:2)<<16;
        int rg=stoi(r2.substr(2))<<20;
        code=0b10010000000000000000000000000000| rc | rg;
        sectionTable[currentSection].offsets.push_back(locationCounter);
        sectionTable[currentSection].data.push_back(code &0xff);
         sectionTable[currentSection].data.push_back(0xff&(code>>8));
        sectionTable[currentSection].data.push_back(0xff&(code>>16));
        sectionTable[currentSection].data.push_back(0xff&(code>>24));
        locationCounter+=4;
    }
    else if(instr=="csrwr"){
        int rg=stoi(r1.substr(2))<<16;
        int rc=(r2=="%status"? 0:r2=="%handler"? 1:2)<<20;
        code=0b10010100000000000000000000000000| rc | rg;
        sectionTable[currentSection].offsets.push_back(locationCounter);
        sectionTable[currentSection].data.push_back(code &0xff);
         sectionTable[currentSection].data.push_back(0xff&(code>>8));
        sectionTable[currentSection].data.push_back(0xff&(code>>16));
        sectionTable[currentSection].data.push_back(0xff&(code>>24));
        locationCounter+=4;
    }
    operand.clear();

}
void Assembler::instructionSecondPassPool(string instr,string r1,string r2,int lineNumber){
    int code=0;
    if(r1[1]=='s') r1="%r14";
    else if(r1[1]=='p') r1="%r15";
    if(r2[1]=='s') r2="%r14";
    else if(r2[1]=='p') r2="%r15";
    map<string,vector<PoolOfLiterals>>::iterator it=pool.find(currentSection);
    if(instr=="call " || instr=="jmp " || instr=="beq " || instr=="bne " || instr=="bgt "){
        long long pom;
        bool isPool;
        if(operand.symbol==""){
            if(operand.literal<lowLimit || operand.literal>highLimit){
                for(auto elem:it->second){
                    if(elem.isConst && elem.value==operand.literal){
                        pom=(elem.adress-locationCounter-4)&0xfff;
                        isPool=true;
                        break;
                    }
                }
            }
            else{
                pom=operand.literal&0xfff;
                isPool=false;
            }
        }
        else{
            if(symbolTable[operand.symbol].section=="ABS" && helpTable[operand.symbol].type=="ABS"){
                if(symbolTable[operand.symbol].value<lowLimit || symbolTable[operand.symbol].value>highLimit){
                for(auto elem:it->second){
                    if(!elem.isConst && elem.symbol==operand.symbol){
                        pom=(elem.adress-locationCounter-4)&0xfff;
                        isPool=true;
                        break;
                    }
                }
                }
                else{
                    pom=symbolTable[operand.symbol].value&0xfff;
                    isPool=false;
                }
            }
            else{
                for(auto elem:it->second){
                    if(!elem.isConst && elem.symbol==operand.symbol){
                        pom=(elem.adress-locationCounter-4)&0xfff;
                        isPool=true;
                        break;
                    }
                }
            }
        }
        
        if(isPool){
            if(instr=="call "){
                code=0b00100001111100000000000000000000|pom;
            }
            else if(instr=="jmp "){
                code=0b00111000111100000000000000000000|pom;
            }
            else if(instr=="beq "){
                int rg1=stoi(r1.substr(2))<<16;
                int rg2=stoi(r2.substr(2))<<12;
                code=0b00111001111100000000000000000000|pom|rg2|rg1;

            }
            else if(instr=="bne "){
                int rg1=stoi(r1.substr(2))<<16;
                int rg2=stoi(r2.substr(2))<<12;
                code=0b00111010111100000000000000000000|pom|rg2|rg1;
            }
            else if(instr=="bgt "){
                int rg1=stoi(r1.substr(2))<<16;
                int rg2=stoi(r2.substr(2))<<12;
                code=0b00111011111100000000000000000000|pom|rg2|rg1;
            }
        }
        else{
              if(instr=="call "){
                code=0b00100000000000000000000000000000|pom;
            }
            else if(instr=="jmp "){
                code=0b00110000000000000000000000000000|pom;
            }
            else if(instr=="beq "){
                int rg1=stoi(r1.substr(2))<<16;
                int rg2=stoi(r2.substr(2))<<12;
                code=0b00110001000000000000000000000000|pom|rg2|rg1;

            }
            else if(instr=="bne "){
                int rg1=stoi(r1.substr(2))<<16;
                int rg2=stoi(r2.substr(2))<<12;
                code=0b00110010000000000000000000000000|pom|rg2|rg1;
            }
            else if(instr=="bgt "){
                int rg1=stoi(r1.substr(2))<<16;
                int rg2=stoi(r2.substr(2))<<12;
                code=0b00110011000000000000000000000000|pom|rg2|rg1;
            }
        }
        sectionTable[currentSection].offsets.push_back(locationCounter);
        sectionTable[currentSection].data.push_back(code &0xff);
         sectionTable[currentSection].data.push_back(0xff&(code>>8));
        sectionTable[currentSection].data.push_back(0xff&(code>>16));
        sectionTable[currentSection].data.push_back(0xff&(code>>24));
        locationCounter+=4;
    }
    else if(instr=="ld"){
        if(operand.reg!=-1){
            if(operand.symbol!="" && (symbolTable[operand.symbol].isDefined==false || symbolTable[operand.symbol].section!="ABS" || helpTable[operand.symbol].type!="ABS" || symbolTable[operand.symbol].value<lowLimit || symbolTable[operand.symbol].value>highLimit)){
                cout<<"Error, symbol must be defined and literal or symbol must be 12b or less, line:"<<(lineNumber-1)<<endl;
                exit(1);
            }
           if(operand.isDir){
                if(operand.symbol==""){
                    int rg=stoi(r1.substr(2))<<20;
                    code=0b10010010000000000000000000000000|rg|(operand.reg<<16)|(operand.literal & 0xfff);
                }
                else{
                    int rg=stoi(r1.substr(2))<<20;
                    code=0b10010010000000000000000000000000|rg|(operand.reg<<16)|(symbolTable[operand.symbol].value&0xfff);
                }
           } 
           else{
            int rg=stoi(r1.substr(2))<<20;
            code=0b10010001000000000000000000000000|rg|(operand.reg<<16);
           }
        }
        else{
            if(operand.isDir){
                long long pom;
                bool isPool;
                if(operand.symbol==""){
                    if(operand.literal<lowLimit || operand.literal>highLimit){
                        for(auto elem:it->second){
                            if(elem.isConst && elem.value==operand.literal){
                                pom=(elem.adress-locationCounter-4)&0xfff;
                                isPool=true;
                                int rg=stoi(r1.substr(2))<<20;
                                code=0b10010010000011110000000000000000|pom|rg;
                                 sectionTable[currentSection].offsets.push_back(locationCounter);
                               sectionTable[currentSection].data.push_back(code &0xff);
                                 sectionTable[currentSection].data.push_back(0xff&(code>>8));
                              sectionTable[currentSection].data.push_back(0xff&(code>>16));
                               sectionTable[currentSection].data.push_back(0xff&(code>>24));
                                code=0b10010010000000000000000000000000|rg|(rg>>4);
                               sectionTable[currentSection].data.push_back(code &0xff);
                                 sectionTable[currentSection].data.push_back(0xff&(code>>8));
                              sectionTable[currentSection].data.push_back(0xff&(code>>16));
                               sectionTable[currentSection].data.push_back(0xff&(code>>24));
                               locationCounter+=8;
                               operand.clear();
                               return;
                                
                                }
                        }
                    }
                    else{
                        pom=operand.literal&0xfff;
                        isPool=false;
                        int rg=stoi(r1.substr(2))<<20;
                        code=0b10010010000000000000000000000000|pom|rg;
                    }
                }
                else{
                    for(auto elem:it->second){
                    if(!elem.isConst && elem.symbol==operand.symbol){
                        pom=(elem.adress-locationCounter-4)&0xfff;
                        isPool=true;
                        int rg=stoi(r1.substr(2))<<20;
                            code=0b10010010000011110000000000000000|pom|rg;
                             sectionTable[currentSection].offsets.push_back(locationCounter);
                           sectionTable[currentSection].data.push_back(code &0xff);
                             sectionTable[currentSection].data.push_back(0xff&(code>>8));
                          sectionTable[currentSection].data.push_back(0xff&(code>>16));
                           sectionTable[currentSection].data.push_back(0xff&(code>>24));
                            code=0b10010010000000000000000000000000|rg|(rg>>4);
                           sectionTable[currentSection].data.push_back(code &0xff);                                
                            sectionTable[currentSection].data.push_back(0xff&(code>>8));
                           sectionTable[currentSection].data.push_back(0xff&(code>>16));
                           sectionTable[currentSection].data.push_back(0xff&(code>>24));
                           locationCounter+=8;
                           operand.clear();
                           return;
                        }
                    }
                }
            }
            else{
                long long pom;
                bool isPool;
                if(operand.symbol==""){
                    if(operand.literal<lowLimit || operand.literal>highLimit){
                        for(auto elem:it->second){
                            if(elem.isConst && elem.value==operand.literal){
                                pom=(elem.adress-locationCounter-4)&0xfff;
                                isPool=true;
                                int rg=stoi(r1.substr(2))<<20;
                                code=0b10010010000011110000000000000000|pom|rg;
                                break;
                                }
                        }
                    }
                    else{
                        pom=operand.literal&0xfff;
                        isPool=false;
                        int rg=stoi(r1.substr(2))<<20;
                        code=0b10010001000000000000000000000000|pom|rg;
                    }
                }
                else{
                
                     if(symbolTable[operand.symbol].section=="ABS" && helpTable[operand.symbol].type=="ABS"){
                        if(symbolTable[operand.symbol].value<lowLimit || symbolTable[operand.symbol].value>highLimit){
                        for(auto elem:it->second){
                            if(!elem.isConst && elem.symbol==operand.symbol){
                                pom=(elem.adress-locationCounter-4)&0xfff;
                                isPool=true;
                                int rg=stoi(r1.substr(2))<<20;
                                code=0b10010010000011110000000000000000|pom|rg;
                                 break;
                              }
                     }
                      }
                     else{
                          pom=symbolTable[operand.symbol].value&0xfff;
                         isPool=false;
                         int rg=stoi(r1.substr(2))<<20;
                        code=0b10010001000000000000000000000000|pom|rg;
                     }
                  }
                  else{
                       for(auto elem:it->second){
                            if(!elem.isConst && elem.symbol==operand.symbol){
                                pom=(elem.adress-locationCounter-4)&0xfff;
                              isPool=true;
                              int rg=stoi(r1.substr(2))<<20;
                                code=0b10010010000011110000000000000000|pom|rg;
                                break;
                         }
                      }
                    }
                    }
                 }
        }
         sectionTable[currentSection].offsets.push_back(locationCounter);
          sectionTable[currentSection].data.push_back(code &0xff);
            sectionTable[currentSection].data.push_back(0xff&(code>>8));
            sectionTable[currentSection].data.push_back(0xff&(code>>16));
           sectionTable[currentSection].data.push_back(0xff&(code>>24));
           locationCounter+=4;
    }
    else if(instr=="st"){
        if(operand.reg!=-1){
            if(operand.symbol!="" && (symbolTable[operand.symbol].isDefined==false || symbolTable[operand.symbol].section!="ABS" || helpTable[operand.symbol].type!="ABS" || symbolTable[operand.symbol].value<lowLimit || symbolTable[operand.symbol].value>highLimit)){
                cout<<"Error, symbol must be defined and literal or symbol must be 12b or less, line:"<<(lineNumber-1)<<endl;
                exit(1);
            }
           if(operand.isDir){
                if(operand.symbol==""){
                    int rg=stoi(r1.substr(2))<<12;
                    code=0b10000000000000000000000000000000|rg|(operand.reg<<20)|(operand.literal&0xfff);
                }
                else{
                    int rg=stoi(r1.substr(2))<<12;
                    code=0b10000000000000000000000000000000|rg|(operand.reg<<20)|(symbolTable[operand.symbol].value&0xfff);
                }
           } 
           else{
            int rg=stoi(r1.substr(2))<<16;
            code=0b10010001000000000000000000000000|rg|(operand.reg<<20);
           }
        }
        else{
            if(operand.isDir){
                long long pom;
                if(operand.symbol==""){
                    if(operand.literal<lowLimit || operand.literal>highLimit){
                         for(auto elem:it->second){
                            if(elem.isConst && elem.value==operand.literal){
                                pom=(elem.adress-locationCounter-4)&0xfff;
                                int rg=stoi(r1.substr(2))<<12;
                                 code=0b10000010111100000000000000000000|rg|pom;
                                 break;
                            }
                         }
                    }
                    else{
                        pom=operand.literal&0xfff;
                        int rg=stoi(r1.substr(2))<<12;
                        code=0b10000000000000000000000000000000|rg|pom;
                    }
                }
                else{
                    if(symbolTable[operand.symbol].section=="ABS" && helpTable[operand.symbol].type=="ABS"){
                        if(symbolTable[operand.symbol].value<lowLimit || symbolTable[operand.symbol].value>highLimit){
                        for(auto elem:it->second){
                            if(!elem.isConst && elem.symbol==operand.symbol){
                                pom=(elem.adress-locationCounter-4)&0xfff;
                                int rg=stoi(r1.substr(2))<<12;
                                code=0b10000010111100000000000000000000|pom|rg;
                                 break;
                              }
                     }
                      }
                     else{
                          pom=symbolTable[operand.symbol].value&0xfff;
                         int rg=stoi(r1.substr(2))<<12;
                        code=0b10000000000000000000000000000000|pom|rg;
                     }
                    }
                    else{
                        for(auto elem:it->second){
                            if(!elem.isConst && elem.symbol==operand.symbol){
                                pom=(elem.adress-locationCounter-4)&0xfff;
                               int rg=stoi(r1.substr(2))<<12;
                                code=0b10000010111100000000000000000000|pom|rg;
                                break;
                         }
                      }
                    }
                }
            }
            else{
                cout<<"Error on line:"<<(lineNumber-1)<<endl;
                exit(1);
            }
        }
         sectionTable[currentSection].offsets.push_back(locationCounter);
          sectionTable[currentSection].data.push_back(code &0xff);
            sectionTable[currentSection].data.push_back(0xff&(code>>8));
            sectionTable[currentSection].data.push_back(0xff&(code>>16));
           sectionTable[currentSection].data.push_back(0xff&(code>>24));
           locationCounter+=4;
    }
    operand.clear();
    
}
void Assembler::createOutputFile(){

    ofstream helpFile1("help_"+outputPath);
    helpFile1 << "Relocative object file" << endl
                     << endl;

    helpFile1 << "Section table:" << endl;
    helpFile1 << "Id\tName\t\tSize" << endl;
    for (map<string, SectionTableEntry>::iterator it = sectionTable.begin(); it != sectionTable.end(); it++)
    {
        helpFile1 <<hex<< (it->second.rnumber&0xf) << "\t\t" << it->second.name << "\t\t\t" << hex << setfill('0') << setw(8) << (0xffffffff & it->second.size) << endl;
    }
    helpFile1 << dec;
    helpFile1 << endl
                     << endl;

    helpFile1 << "Symbol table:" << endl;
    helpFile1 << "Value\t\tType\tSection\t\tId\tName" << endl;
    for (map<string, SymbolTableEntry>::iterator it = symbolTable.begin(); it != symbolTable.end(); it++)
    {
        helpFile1 << hex << setfill('0') << setw(8) << (0xffffffff & it->second.value) << "\t";
        if (it->second.isLocal == true)
            helpFile1 << "l\t\t";
        else
        {
            if (it->second.isDefined == true)
                helpFile1 << "g\t\t";
            else
            {
                if (it->second.isExtern)
                    helpFile1 << "e\t\t";
               
            }
        }
        helpFile1 << it->second.section << "\t\t\t" << hex << setfill('0') << setw(4) << (0xffff & it->second.rnumber) <<"\t" << it->second.name << endl;
    }
    helpFile1 << dec;
    helpFile1<< endl
                     << endl;
    for (map<string, SectionTableEntry>::iterator it = sectionTable.begin(); it != sectionTable.end(); it++)
    {
        helpFile1 << "Relocation data <" << it->first << ">:" << endl;
        helpFile1<< "Offset\t\tDat/Equ\tSection\tAddend\tSymbol" << endl;
        for (RelocationTableEntry rel : relocationTable[it->first])
        {
            
            helpFile1 << hex << setfill('0') << setw(8) << (0xffffffff & rel.offset) << "\t" << (rel.isData ? 'd' : 'e') << "\t\t\t\t"  << rel.section << "\t\t\t"<<hex<<setfill('0')<<setw(4)<< (rel.addend&0xffff)<<"\t"<< rel.symbol << "\t"<<endl;
        }
        helpFile1 << dec << endl;

        helpFile1 << "Section data <" << it->first << ">:" << endl;

        SectionTableEntry section = it->second;
        if (section.size == 0)
        {
            helpFile1 << dec;
            helpFile1 << endl
                             << endl;
            continue;
        }
        int counter = 0;

        for (int i = 0; i < section.offsets.size() - 1; i++)
        {

            int currOffset = section.offsets[i];
            int nextOffset = section.offsets[i + 1];
            helpFile1 << hex << setfill('0') << setw(8) << (0xffffffff & currOffset) << ": ";
            for (int j = currOffset; j < nextOffset; j++)
            {
                char c = section.data[j];
                helpFile1 << hex << setfill('0') << setw(2) << (0xff & c) << " ";
            }
            helpFile1 << endl;
        }
        int currOffset = section.offsets[section.offsets.size() - 1];
        int nextOffset = section.data.size();
        helpFile1 << hex << setfill('0') << setw(8) << (0xffffffff & currOffset) << ": ";
        for (int j = currOffset; j < nextOffset; j++)
        {
            char c = section.data[j];
            helpFile1 << hex << setfill('0') << setw(2) << (0xff & c) << " ";
        }
        helpFile1 << dec;
        helpFile1 << endl
                         << endl;
    }
    helpFile1.close();
}

void Assembler::createBinaryFile()
{
    string filename = "./" + outputPath;
    ofstream binaryFile(filename, ios::out | ios::binary);

    

    int numOfSymbols = symbolTable.size();
    binaryFile.write((char *)&numOfSymbols, sizeof(numOfSymbols));
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
