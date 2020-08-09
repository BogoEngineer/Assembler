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
    /*cout<< "TABLE: "<<endl;
    for(SymbolTableEntry ste: table){
        cout<<ste.name<< " "<<ste.offset<<endl;
    }*/
}

void SymbolTableEntry::addForwardReference(ForwardReferenceTableEntry frte){
    forward_reference_table.push_back(frte);
}

void SymbolTableEntry::resolveSymbol(vector<char>* machine_code, string section_name){
    short int symbol = this->offset;
    for(ForwardReferenceTableEntry frte: forward_reference_table){
        if(frte.section != section_name) continue;
        cout<<"PATCH: "<<frte.byte;
        (*machine_code)[frte.byte + 1] =  (char)(symbol & 0xFF);
        (*machine_code)[frte.byte] = (char)(symbol & 0xFF00)>>8;
        cout<< " VALUE: "<< symbol<<endl;
    }
}

void SymbolTable::backpatch(vector<char>& machine_code, string section_name){
    for(SymbolTableEntry ste: table){
        ste.resolveSymbol(&machine_code, section_name);
    }
}