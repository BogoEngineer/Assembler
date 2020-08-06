#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include "INCLUDES.h"
#include <vector>

#include "ForwardReferenceTable.h"

struct SymbolTableEntry{
    static int global_id;
    string name;
    int id;
    string section;
    int offset;
    bool local;
    bool defined;
    ForwardReferenceTable* flink;

    SymbolTableEntry(string n, string s, int o, bool l, bool d): 
    name(n), section(s), offset(o), local(l), defined(d){
        id = global_id;
        global_id += 1;
    }
};

struct find_symbol : std::unary_function<SymbolTableEntry, bool> {
    string name;
    find_symbol(string n):name(n) { }
    bool operator()(SymbolTableEntry const& ste) const {
        return ste.name == name;
    }
};

class SymbolTable{
    public:
        vector<SymbolTableEntry> table;
        SymbolTableEntry* findSymbol(string symbol);
        void addSymbol(SymbolTableEntry ste);
};

#endif