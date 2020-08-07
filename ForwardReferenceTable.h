#ifndef FORWARDREFERENCETABLE_H
#define FORWARDREFERENCETABLE_H
#include "INCLUDES.h" 
#include <vector>
#include "SymbolTable.h"

struct SymbolTableEntry;

struct ForwardReferenceTableEntry{
    int patch;
    int byte;

    ForwardReferenceTableEntry(int b, int p = 0): byte(b), patch(p){}
};

class ForwardReferenceTable{
    public:
        vector<ForwardReferenceTableEntry> table;
        
        ForwardReferenceTable(){ table = {};}

        void addForwardReference(ForwardReferenceTableEntry frte);

        void resolveSymbol(SymbolTableEntry ste);
};

#endif