#ifndef EMULATOR_H
#define EMULATOR_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

class Emulator
{
      enum InstructionMnemonic
    {
        halt,
        intt,
        iret,
        call,
        ret,
        jmp,
        beq,
        bne,
        bgt,
        push,
        pop,
        xchg,
        add,
        sub,
        mul,
        div,
        nott,
        andd,
        orr,
        xorr,
        shl,
        shr,
        ld,
        st,
        csrrd,
        csrwr
    };

    enum Register
    {
        r0 = 0,
        r1 = 1,
        r2 = 2,
        r3 = 3,
        r4 = 4,
        r5 = 5,
        r6 = 6,
        r7 = 7,
        r8 = 8,
        r9 = 9,
        r10 = 10,
        r11 = 11,
        r12 = 12,
        r13 = 13,
        r14 = 14, 
        r15 = 15,
        rsp = r14,
        rpc = r15,
        rstatus = 16,
        rhandler = 17,
        rcause = 18
    };

  
    enum AddressType
    {
        pool,
        noPool,
        pushh,
        popp,
        regG,
        regS,
        regGwithDisp,
        memDir,
        memDirS
    };
     enum Flag
    {
        Tr = 1,
        Tl = 1 << 1,
        I = 1 << 2,
        FAULT=1,
        TIMER=2,
        TERMINAL=3,
        INTT=4
    };

    struct Segment
    {
        int adress; 
        int size;
        vector<char> data; 
    };
     bool fetch();
    bool execute();
    static int START_ADRESS;
    map<int,char> memory;                                 
    int read(int,int=WORD);         
    void write(int, int,int=WORD); 
    vector<Segment> segments;
    string inputFile;

    bool running; 

    InstructionMnemonic instruction; 
    int regA;
    int regB;
    int regC;
    int disp;
    AddressType mod;          

    bool instructionDone=true;
    int pastPC; 

   

    vector<int> registers; 
    int &pc = registers[rpc];

    int &sp = registers[rsp];
    void pushStack(int);
    int popStack();

    int &status = registers[rstatus];
    int &handler = registers[rhandler];
    int &cause = registers[rcause];
    
    vector<bool> intRequest; 
    
   

    
    int timerPeriodId; 
    long long int timerPeriod;       

    bool counting; 

    long long int pastTime;
    long long int time;

    long long int duration(int); 
    void resetTimer();
    void timerTick();

    void clcFlag(int);
    void setFlag(int);
    int getFlag(int);
     static int TIMER_REQ;    
    static int TERM_OUT; 
    static int TERM_IN;  
    static int TIM_CFG;  
    void configureTerminal();
    void resetTerminal();
    void readCharacters();
     void setInterruptRequestOnLine(int); 
    void interruptHandler();       
    void interrupt();  

public:
    Emulator(string);
    void collectData();

    void emulate();

    static long long MEMORY;            
    static unsigned int MAPPED_REG; 
    static int NUMBER_OF_REG;         
    static int WORD;                   


    static int NUMBER_OF_PERIFERIES; 
    static int TERMINAL_REQ; 
   
};

#endif //EMULATOR_H