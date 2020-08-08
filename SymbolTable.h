#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include "INCLUDES.h"
#include <vector>

struct ForwardReferenceTableEntry{
    int size;
    int byte;

    ForwardReferenceTableEntry(int b, int s = 2): byte(b), size(s){}
};

struct SymbolTableEntry{
public:
    static int global_id;
    string name;
    int id;
    string section;
    short int offset;
    bool local;
    bool defined;
    vector<ForwardReferenceTableEntry> forward_reference_table;

    SymbolTableEntry(string n, string s="", short int o=0, bool l=0, bool d=false): 
    name(n), section(s), offset(o), local(l), defined(d){
        id = global_id;
        global_id += 1;
        forward_reference_table = {};
    }

    void addForwardReference(ForwardReferenceTableEntry frte);

    void resolveSymbol(vector<char>* machine_code);
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
        void backpatch(vector<char>* machine_code);
};

#endif