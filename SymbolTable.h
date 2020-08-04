#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include "INCLUDES.h"

#include "SymbolTableEntry.h"
#include <vector>

class SymbolTable{
    private:
        vector<SymbolTableEntry> table;
};

#endif