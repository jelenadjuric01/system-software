#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <termios.h>

#include "../inc/emulatorHelper.hpp"

using namespace std;

long long Emulator::MEMORY = 1LL << 32;
unsigned int Emulator::MAPPED_REG = 0xFFFF0000;
int Emulator::NUMBER_OF_REG = 19;
int Emulator::WORD = 4; 
int Emulator::TERM_OUT = 0xFFFFFF00; 
int Emulator::TERM_IN = 0xFFFFFF04; 
int Emulator::TIM_CFG = 0XFFFFFF10; 
int Emulator::START_ADRESS=0x40000000;

int Emulator::NUMBER_OF_PERIFERIES = 2; 
int Emulator::TERMINAL_REQ = 0;
int Emulator::TIMER_REQ = 1;    

 

Emulator::Emulator(string file)
    : inputFile(file), registers(NUMBER_OF_REG, 0),
      running(false),memory(),
      intRequest(NUMBER_OF_PERIFERIES, false), 
      counting(false)
{
    pastTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    
    
}

void Emulator::collectData()
{

    ifstream input(inputFile, ios::binary);
    if (input.fail())
    {
        cout<<"File " + inputFile + " can not be opened!"<<endl;
        exit(1);
    }

    int numOfSeg = 0;
    input.read((char *)&numOfSeg, sizeof(numOfSeg));
    
    for (int i = 0; i < numOfSeg; i++)
    {
        Segment segment;
        long long addr;
        input.read((char *)&addr, sizeof(addr));
        segment.adress=(int)addr;
        input.read((char *)&segment.size, sizeof(segment.size));
        for (int j = 0; j < segment.size; j++)
        {
            char c;
            input.read((char *)(&c), sizeof(c));
            segment.data.push_back(c);
        }
        int counter = 0;

        for (int i = 0; i < segment.data.size(); i++)
        {
            char c = segment.data[i];
           
            counter++;
            
        }
        segments.push_back(segment);
    }
    input.close();
    for (Segment s : segments)
    {
        for (int i = 0; i < s.data.size(); i++)
        {
            if (i + s.adress > MAPPED_REG)
            {   cout<<MAPPED_REG<<endl;
                cout<<"Virtual address exceeds maximal user available memory "<<(i+s.adress)<<endl;
                exit(1);
            }
            memory[i + s.adress] = s.data[i];
        }
    }

}


int Emulator::read(int address,int size) 
{
    if(size==1){
        return memory[address];
    }
        char first = memory[address];
        char second = memory[address + 1];
        char third = memory[address+2];            
        char fourth=memory[address+3];
        
        int value = (int)(((fourth<<24)&0xff000000)| ((third<<16)&0x00ff0000) | ((second <<8)&0x0000ff00) | (0xff & first));
        return value;
        
    
}

void Emulator::write(int value, int address,int size)
{
    if(size==1){
        memory[address]=value &0xff;
    }
        char first = value & 0xff;
        char second = (value >> 8) & 0xff;
        char third=(value>>16)&0xff;
        char fourth = (value>>24)&0xff;
            memory[address] = first;
            memory[address + 1] = second;
            memory[address+2]=third;
            memory[address+3]=fourth;

    if (address == TERM_OUT)
    {
        cout << (char)value << flush;
    }
}

void Emulator::readCharacters()
{
    char c;

    if (::read(STDIN_FILENO, &c, 1) == 1)
    {
        write(c, TERM_IN, WORD);
        setInterruptRequestOnLine(TERMINAL_REQ);
    }

}
void Emulator::emulate()
{

    pc = START_ADRESS;
    sp = MAPPED_REG;
    handler=MAPPED_REG;
    
   clcFlag(Tr);
   clcFlag(Tl);
   clcFlag(I);
    resetTimer(); 
    configureTerminal();
    instructionDone=true;
    running = true;

    while (running)
    {
        pastPC = pc;

        if (fetch() == false)
        {
            pc = pastPC;
            cause=FAULT;
            interrupt();
        }
        else{
            if (execute() == false)
        {
            pc = pastPC;
            cause=FAULT;
            interrupt();
        }
        }
        if(instructionDone){
        timerTick();
        readCharacters();
        interruptHandler();
        }

    }

    resetTerminal();
    cout<<endl;
    cout << "------------------------------------------------" << endl;
    cout << "Emulated processor executed halt instruction" << endl;
    cout << "Emulated processor state:" << endl;
  
  cout << "r0=0x" << std::setfill('0') << std::setw(8) << hex << (int)registers[r0] << " " ;
  cout << "r1=0x" << std::setfill('0') << std::setw(8) << hex <<  (int)registers[r1] << " " ;
  cout << "r2=0x" << std::setfill('0') << std::setw(8) << hex <<  (int)registers[r2] << " " ;
  cout << "r3=0x" << std::setfill('0') << std::setw(8) << hex <<  (int)registers[r3] << " " ;
  cout << endl;
  cout << "r4=0x" << std::setfill('0') << std::setw(8) << hex <<  (int)registers[r4] << " " ;
  cout << "r5=0x" << std::setfill('0') << std::setw(8) << hex <<  (int)registers[r5] << " " ;
  cout << "r6=0x" << std::setfill('0') << std::setw(8) << hex <<  (int)registers[r6] << " " ;
  cout << "r7=0x" << std::setfill('0') << std::setw(8) << hex <<  (int)registers[r7] << " " ;
  cout<<endl;
  cout << "r8=0x" << std::setfill('0') << std::setw(8) << hex <<  (int)registers[r8] << " " ;
  cout << "r9=0x" << std::setfill('0') << std::setw(8) << hex <<  (int)registers[r9] << " " ;
  cout << "r10=0x" << std::setfill('0') << std::setw(8) << hex <<  (int)registers[r10] << " " ;
  cout << "r11=0x" << std::setfill('0') << std::setw(8) << hex <<  (int)registers[r11] << " " ;
  cout<<endl;
  cout << "r12=0x" << std::setfill('0') << std::setw(8) << hex <<  (int)registers[r12] << " " ;
  cout << "r13=0x" << std::setfill('0') << std::setw(8) << hex <<  (int)registers[r13] << " " ;
  cout << "r14=0x" << std::setfill('0') << std::setw(8) << hex <<  (int)registers[r14] << " " ;
  cout << "r15=0x" << std::setfill('0') << std::setw(8) << hex <<  (int)registers[r15] << " " ;
    cout<<endl;



}
void Emulator::interrupt()
{
    if(handler==MAPPED_REG) return;

    pushStack(status);
    pushStack(pc);

    pc = handler;

    setFlag(I);
    setFlag(Tr);
    setFlag(Tl);
}
bool Emulator::fetch()
{
    int instr=read(pc,WORD);
    char code = (instr>>28) &0xf;
    char modificator = (instr>>24) & 0xf;
    regA=(instr>>20)&0xf;
    regB=(instr>>16)&0xf;
    regC=(instr>>12)&0xf;
    int d=(int)((instr&0xfff)|((instr>>11)&1==1?0xfffff000:00000000));
    disp=d;
    pc+=4;
   switch (code)
    {
    case 0x0:
    {

        instruction = halt;
        if (modificator != 0x0 || regA!=0 || regB!=0 || regC!=0 || disp!=0)
        {
            cout<<"Instruction does not exists!"<<endl;
            return false;
        }
        return true;
    }
    case 0x1:
    {
        instruction = intt;

        if (modificator != 0x0 || regA!=0 || regB!=0 || regC!=0 || disp!=0)
        {
            cout<<"Instruction does not exists!"<<endl;
            return false;
        }
        return true;
    }
    case 0x2:
    {
        
        instruction = call;

        if (regC != 0)
        {
            cout<<"Instruction does not exists!"<<endl;
            return false;
        }
        if(modificator== 0x0){
            mod=noPool;
        }
        else if(modificator==0x1){
            mod=pool;
        }
        else{
            cout<<"Instruction does not exists!"<<endl;
            return false;
        }

        return true;
    }
    case 0x3:
    {
        if(instr==0b00111000111000000000111111111000) instructionDone=true;
        switch(modificator){
            case 0x0:
            case 0x8:
            {
                instruction=jmp;
                if(regB!=0 || regC!=0){
                    cout<<"Instruction does not exists!"<<endl;
                    return false;
                }
                if(modificator== 0x0){
            mod=noPool;
                }
            if(modificator==0x0){
                mod=noPool;
            }else{
                mod=pool;
            }
           
            }
            break;
            case 0x1:
            case 0x9:{
                instruction=beq;
                if(modificator==0x1){
                    mod=noPool;
                }
                else{
                    mod=pool;
                }
                
            }
            break;
            case 0x2:
            case 0xa:{
                instruction=bne;
                if(modificator==0x2){
                    mod=noPool;
                }
                else{
                    mod=pool;
                }
            }
            break;
            case 0x3:
            case 0xb:
            {
                instruction=bgt;
                if(modificator==0x3){
                    mod=noPool;
                }
                else{
                    mod=pool;
                }
            }
            break;
            default:{
                cout<<"Instruction does not exists!"<<endl;
                return false;
            }
            return true;
        }
        break;
    }
    case 0x4:
    {
        instruction = xchg;
        if (disp != 0 || modificator!=0x0 || regA!=0)
        {
            cout<<"Instruction does not exists!"<<endl;
            return false;
        }

        return true;
    }
    case 0x5:
    {
        switch (modificator)
        {
        case 0x0:
            instruction = add;
            break;
        case 0x1:
            instruction = sub;
            break;
        case 0x2:
            instruction = mul;
            break;
        case 0x3:
            instruction = div;
            break;

        default:
            cout<<"Instruction does not exists!"<<endl;
                return false;
        } 
        if(disp!=0){
            cout<<"Instruction does not exists!"<<endl;
               return false;
        }

        return true;
    }
    case 0x6:
    {
        switch (modificator)
        {
        case 0x0:
{            instruction = nott;
            if(regC!=0){
                cout<<"Instruction does not exists!"<<endl;
                return false;
            }
}            break;
        case 0x1:
            instruction = andd;
            break;
        case 0x2:
            instruction = orr;
            break;
        case 0x3:
            instruction = xorr;
            break;

        default:
            cout<<"Instruction does not exists!"<<endl;
               return false;
        } 
        if(disp!=0){
            cout<<"Instruction does not exists!"<<endl;
               return false;
        }

        return true;
    }
    case 0x7:
    {
        switch (modificator)
        {
        case 0x0:
            instruction = shl;
            break;
        case 0x1:
            instruction=shr;
            break;
        default:
        cout<<"Instruction does not exists!"<<endl;
               return false;

    }
        if(disp!=0){
            cout<<"Instruction does not exists!"<<endl;
            return false;
        }

        return true;
    }
    case 0x8:
    {
        switch (modificator)
        {
        case 0x0:
            instruction = st;
            mod=noPool;
            break;
        case 0x1:
            instruction = st;
            mod=pushh;
            break;
        case 0x2:
            instruction = st;
            mod=pool;
            break;

        default:
            cout<<"Instruction with that operation code and modifier does not exists!"<<endl;
            return false;
        } 
        return true;
    }
    case 0x9:
    {
        instruction=ld;
        switch (modificator)
        {
        case 0x0:
            mod = regG;
            break;
        case 0x1:
            mod = regGwithDisp;
            if(regA==14 && regB==14 && regC==0 && disp==8){
                int next=read(pc);
                if(next==0b10010110000011100000111111111100){
                    if(read(pc+4)==0b00111000111000000000111111111000){
                        instructionDone=false;
                    }
                }
            }
            break;
        case 0x2:
            mod=memDir;
            break;
        case 0x3:
            mod=popp;
            if(regA==15 && regB==14 && regC==0 && disp==4){
                instruction=ret;
            }
            break;
        case 0x4:
            mod=regS;
            break;
        case 0x6:
            mod=memDirS;
        break;
        default:
            cout<<"Dodaj ostala"<<endl;
            return false;
        }

        

        return true;
    }
    
        
    default:

        cout<<"Instruction does not exists!";
        return false;
    }
    return true;
}




bool Emulator::execute()
{
    switch (instruction)
    {
    case halt:
    {
        running = false;
        return true;
    } 

    case intt:
    {

        cause=INTT;
        interrupt();
        return true;
    }
   
     case call:
    {
        

        pushStack(pc);
        int operand;
        if(mod==noPool){
            operand=registers[regA]+registers[regB]+disp;
        }
        else{
            operand=read((registers[regA]+registers[regB]+disp),WORD);
        }
        pc = operand;

        
        return true;
    } 
    case ret:
    {
         
        pc = popStack();
        return true;
    }
    case jmp:
    {
        

        pc = (mod==pool?read((registers[regA]+disp),WORD):(registers[regA]+disp));

        return true;
    }
    case beq:
    {
        
       
        int operand = (mod==pool?read((registers[regA]+disp),WORD):(registers[regA]+disp));


        if (registers[regB]==registers[regC])
        {
            pc = operand;
        }
        

        return true;
    }
    case bne:
    {

        int operand = (mod==pool?read((registers[regA]+disp),WORD):(registers[regA]+disp));

        
        if (registers[regB]!=registers[regC])
        {
            pc = operand;
        }

        return true;
    }
    case bgt:
    {
       int operand = (mod==pool?read((registers[regA]+disp),WORD):(registers[regA]+disp));

        
        if (registers[regB]>registers[regC])
        {
            pc = operand;
        }
        

        return true;
    }
    case xchg:
    {
       
        int tmp = registers[regA];
        registers[regA] = registers[regB];
        registers[regB] = tmp;

        return true;
    }

    case add:
    {
       
        registers[regA] = registers[regC] + registers[regB];
        return true;
    }
    case sub:
    {
        
        registers[regA] = registers[regB] - registers[regC];

       
        return true;
    }
    case mul:
    {
        
        registers[regA] = registers[regC] * registers[regB];

        return true;
    }
    case div:
    {
       
       if (registers[regB] == 0)
        {
            cout << "DIVISION WITH ZERO! ";
            break;
        }
        registers[regA] = registers[regB] / registers[regC];

       
        return true;
    }
    case nott:
    {
       

        registers[regA] = !registers[regB];

        return true;
    }
    case andd:
    {
       
        registers[regA] = registers[regC] & registers[regB];

        return true;
    }
    case orr:
    {
        
        registers[regA] = registers[regC] | registers[regB];

     
        return true;
    }
    case xorr:
    {
       
        registers[regA] = registers[regC] ^ registers[regB];

      
        return true;
    }
    case shl:
    {
       

        registers[regA] = registers[regB] << registers[regC];
        
      
        return true;
    }
    case shr:
    {
        
        registers[regA] = registers[regB] >> registers[regC];
       
        return true;
    }

    case st:
    {
        int operand;
        
        switch(mod){
            case noPool:
            write(registers[regC],(registers[regA]+registers[regB]+disp),WORD);
            break;
            case pool:
            operand=read((registers[regA]+registers[regB]+disp));
            write(registers[regC],operand,WORD);
            break;
            case pushh:
            pushStack(registers[regC]);
            break;
        }


        
        return true;
    } 
    case ld:
    {
       
        int operand;
        switch(mod){
            case regG:
            registers[regA]=registers[16+regB];
            break;
            case regGwithDisp:
            registers[regA]=registers[regB]+disp;
            break;
            case memDir:
            registers[regA]=read((registers[regB]+registers[regC]+disp),WORD);
            break;
            case popp:
            registers[regA]= popStack();
            break;
            case regS:
            registers[regA+16]=registers[regB];
            break;
            case memDirS:
            registers[16+regA]=read((registers[regB]+registers[regC]+disp));
            break;
            default:
            cout<<"DODAJ OSTALE ZA LD"<<endl;
            
        }

        
        return true;
    } 
    default:
    {
        {
            cout << "ERROR ";
            return false;
            break;
        }
    }
    } 
    return true;
}

void Emulator::pushStack(int value)
{
    sp -= 4;
    write(value, sp, WORD);
}
int Emulator::popStack()
{
    int value = read(sp, WORD);
    sp += 4;

    return value;
}

void Emulator::interruptHandler()
{
    if (getFlag(I) == 0)
    {
        for (int interrupt_line_number = 0; interrupt_line_number < NUMBER_OF_PERIFERIES; interrupt_line_number++)
        {
            if (intRequest[interrupt_line_number] == true)
            {
                if (interrupt_line_number == TIMER_REQ)
                {
                    if (getFlag(Tr) == 0)
                    {
                        intRequest[interrupt_line_number] = false;
                        cause=TIMER;
                        interrupt();
                        break;
                    }
                    
                }
                else if (interrupt_line_number == TERMINAL_REQ)
                {
                    if (getFlag(Tl) == 0)
                    {
                        intRequest[interrupt_line_number] = false;
                        cause=TERMINAL;
                        interrupt();
                        break;
                    }
                   
                }
            }
        }
    }
   
}

void Emulator::setInterruptRequestOnLine(int intr)
{

    
        intRequest[intr] = true;
        
}

void Emulator::timerTick()
{

    time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    if (counting == true)
    {
        if (time - pastTime > timerPeriod)
        {
            counting = false;
            setInterruptRequestOnLine(TIMER_REQ);
        }
    }
    if (counting == false)
    {
        timerPeriodId = read(TIM_CFG, WORD);
       
        timerPeriod = duration(timerPeriodId); 

        counting = true;
        pastTime = time;
    }
}

void Emulator::clcFlag(int flag)
{
    status &= ~flag;
}
void Emulator::setFlag(int flag)
{
    status |= flag;
}

int Emulator::getFlag(int flag)
{
    return status & flag;
}
void Emulator::resetTimer()
{
    timerPeriodId = 0x0;
    timerPeriod = duration(timerPeriodId); 
    counting = true;

}

long long int Emulator::duration(int id)
{
    switch (id)
    {
    case 0x0:
        return 500000;
    case 0x1:
        return 1000000;
    case 0x2:
        return 1500000;
    case 0x3:
        return 2000000;
    case 0x4:
        return 5000000;
    case 0x5:
        return 7000000; 
    case 0x6:
        return 30000000; 
    case 0x7:
        return 60000000; 

    default:
        return 500;
    }
}

struct termios stdin_backup_settings; 
   void backup_stdin_settings()
{
	
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &stdin_backup_settings);
}
    

void Emulator::configureTerminal()
{
	
    if (tcgetattr(STDIN_FILENO, &stdin_backup_settings) < 0)
    {
        cout<< "Cannot fetch settings from STDIN_FILENO to save them!"<<endl;
        exit(1);
    }
    static struct termios changed_settings = stdin_backup_settings;
   
    changed_settings.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN);
    
    changed_settings.c_cc[VMIN] = 0;  
    changed_settings.c_cc[VTIME] = 0; 

    changed_settings.c_cflag &= ~(CSIZE | PARENB);
    changed_settings.c_cflag |= CS8;
    if (atexit(backup_stdin_settings) != 0)
    {
        cout<<"Cannot backup settings to STDIN_FILENO!"<<endl;
        exit(1);
    }

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &changed_settings))
    {
        cout<<"Cannot set changed settings to STDIN_FILENO"<<endl;
        exit(1);
    }
}




void Emulator::resetTerminal()
{
    backup_stdin_settings();
}


