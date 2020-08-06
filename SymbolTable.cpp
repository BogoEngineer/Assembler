#include "SymbolTable.h"
#include <algorithm>

int SymbolTableEntry::global_id = 0;

SymbolTableEntry* SymbolTable::findSymbol(string symbol){
    vector<SymbolTableEntry>::iterator it = std::find_if(table.begin(), table.end(), find_symbol(symbol));
    if(it != table.end()) return it.base();
    return nullptr;
}

void SymbolTable::addSymbol(SymbolTableEntry symbol){
    table.push_back(symbol);
}