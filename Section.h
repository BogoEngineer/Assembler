#ifndef SECTION_H
#define SECTION_H
#include "INCLUDES.h"
#include <vector>

struct RelocationTableEntry{
    int offset;
    string type;
    int value;

    RelocationTableEntry(int o, string t, int v):offset(o), type(t), value(v){}
};

class Section{
    public:
        string name;
        vector<char> machine_code;
        vector<RelocationTableEntry> relocation_table;
        int location_counter;

        Section(string n);

        string getMachineCodeString();
        string getRelocationTable();
        
        string byteCodeToString(vector<char> byte_code);

        vector<char>& getMachineCode();

};

#endif