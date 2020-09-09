#ifndef SECTION_H
#define SECTION_H
#include "INCLUDES.h"
#include <vector>

struct RelocationTableEntry{
    int offset;
    string type;
    int value;
    string symbol_name;

    RelocationTableEntry(int o, int v, string t, string syn):offset(o), type(t), value(v), symbol_name(syn){}
};

class Section{
    public:
        string name;
        vector<char> machine_code;
        vector<RelocationTableEntry> relocation_table;
        int location_counter;

        Section(string n);
        ~Section();

        string getMachineCodeString();
        string getRelocationTable();
        
        string byteCodeToString(vector<char> byte_code);

        vector<char>& getMachineCode();

};

#endif