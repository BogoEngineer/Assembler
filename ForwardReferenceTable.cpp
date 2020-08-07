#include "ForwardReferenceTable.h"

void ForwardReferenceTable::addForwardReference(ForwardReferenceTableEntry frte){
    table.push_back(frte);
}

void ForwardReferenceTable::resolveSymbol(SymbolTableEntry ste){

}