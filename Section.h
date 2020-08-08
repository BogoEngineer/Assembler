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

        Section(string n);

        string getMachineCode();
        string getRelocationTable();
};

#endif