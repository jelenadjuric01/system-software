#include <iostream>
#include <fstream>
#include "../inc/linkerHelper.hpp"
#include <regex>
using namespace std;



int main(int argc, const char *argv[])
{

    bool output = false;
    string outputFileName = "linker.o";
    bool place = false;
    regex placeReg("^-place=([a-zA-Z_][a-zA-Z_0-9]*)@(0[xX][0-9a-fA-F]+)$");
    smatch sectionAdress;
    map<string,long long> placeHelper;
    bool hex = false;
    bool relocatable = false;
    vector<string> files;
    for (int i = 1; i < argc; i++)
    {
        string current = argv[i];
        if (current == "-o")
        {

            output = true;
        }
        else if (current == "-hex")
        {
            hex = true;
        }
        else if (current == "-relocatable")
        {
            relocatable = true;
        }
        else if (regex_search(current, sectionAdress, placeReg))
        {
            place = true;
            string section = sectionAdress.str(1);
            long long address = stoll(sectionAdress.str(2), nullptr, 16);
            placeHelper[section] = address;
        }
        else if (output)
        {
            outputFileName = current;
            output = false;
        }
        else
        {
            files.push_back(current);
        }
    }

    if (relocatable == true && hex == true)
    {
        cout << "-relocatable and -hex options are not allowed at the same time!" << endl;
        return -1;
    }
    if (relocatable == false && hex == false)
    {
        cout << "One option -relocatable or -hex has to be set!" << endl;

        return -1;
    }

    Linker linker(outputFileName, files, relocatable, placeHelper);
    linker.collectData();
    linker.connectSections();
    linker.connectSymbolTable();
    linker.connectRelocationTable();
    linker.connectSectionData();
    linker.resolveRelocation();
    linker.createOutputFile();
    linker.createBinaryFile();
    return 0;
}