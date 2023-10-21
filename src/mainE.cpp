#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <regex>

#include "../inc/emulatorHelper.hpp"

using namespace std;

int main(int argc, const char *argv[])
{
    if (argc != 2)
    {
        cout << "Only one file is allowed!" << endl;
        return -1;
    }
    Emulator emulator(argv[1]);

    emulator.collectData();
    emulator.emulate();

    return 0;
}