#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include "INCLUDES.h"
#include <vector>

#include "ForwardReferenceTable.h"

struct SymbolTableEntry{
    string name;
    int id;
    int size;
    int value;
    int section;
    int offset;
    bool local;
    bool defined;
    ForwardReferenceTable* flink;
};

class SymbolTable{
    private:
        vector<SymbolTableEntry> table;
};

#endif