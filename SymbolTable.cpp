#include "SymbolTable.h"
#include <algorithm>
#include <sstream>

int SymbolTableEntry::global_id = 0;

SymbolTableEntry* SymbolTable::findSymbol(string symbol){
    vector<SymbolTableEntry>::iterator it = std::find_if(table.begin(), table.end(), find_symbol(symbol));
    if(it != table.end()) return it.base();
    return nullptr;
}

void SymbolTable::addSymbol(SymbolTableEntry symbol){
    table.push_back(symbol);
}

void SymbolTableEntry::addForwardReference(ForwardReferenceTableEntry frte){
    forward_reference_table.push_back(frte);
}

void SymbolTableEntry::resolveSymbol(vector<char>* machine_code, string section_name){
    SymbolTableEntry *ste = this;
    short int symbol = ste->offset;
    //cout<<"SYMBOL: "<<this->name<<" FRTE: "<<this->forward_reference_table.size()<<endl;
    for(ForwardReferenceTableEntry frte: forward_reference_table){
        if(frte.section != (section_name[0] == '.' ? section_name.substr(1) : section_name)) continue;
        //cout<<" PATCH: "<<frte.byte;
        //cout<<"SEC1: "<<frte.section<<" SEC2: "<<ste->section<<endl;
        int pcrel = frte.pcrel ? ((frte.section == ste->section ? (-frte.byte):0) + frte.end_of_instruction_offset) : 0;
        (*machine_code)[frte.byte] =  (symbol+pcrel) & 0xFF;
        (*machine_code)[frte.byte + 1] = ((symbol+pcrel)>>8) & 0xFF; // little endian
        //cout<< " VALUE: "<< symbol<<endl;
    }
}

void SymbolTable::backpatch(vector<char>& machine_code, string section_name){
    //cout<<"SECTION: "<<section_name;
    for(SymbolTableEntry ste: table){
        if(ste.defined == false && ste.section != "UND") {
            cout<<"Could not resolve symbol: "<<ste.name<<endl;
            exit(1);
        }
        if(ste.local==false) continue; // no need to backpatch for global symbols
        ste.resolveSymbol(&machine_code, section_name);
    }
}

/*! Center-aligns string within a field of width w. Pads with blank spaces
    to enforce alignment. */
string center(const string s, const int w) {
    stringstream ss, spaces;
    int padding = w - s.size();                 // count excess room to pad
    for(int i=0; i<padding/2; ++i)
        spaces << " ";
    ss << spaces.str() << s << spaces.str();    // format with padding
    if(padding>0 && padding%2!=0)               // if odd #, add 1 space
        ss << " ";
    return ss.str();
}

/* Convert double to string with specified number of places after the decimal
   and left padding. */
string prd(const string x, const int decDigits, const int width) {
    stringstream ss;
    ss << fixed << right;
    ss.fill(' ');        // fill space around displayed #
    ss.width(width);     // set  width around displayed #
    ss.precision(decDigits); // set # places after decimal
    ss << x;
    return ss.str();
}

string SymbolTable::toString(){
    stringstream ss;
    ss<< "#SYMBOL TABLE: "<<endl;
    ss  << center("name", 15)<< " | "
        << center("section", 10)<< " | "
        << center("offset", 11)<< "|"
        << center("local", 12)<< "|"
        << center("id", 10)<<endl;
    for(SymbolTableEntry ste: table){
        ss<< prd(ste.name,1,15)       << " | "
          << prd(ste.section,1,10)       << " | "
          << prd(to_string(ste.offset),1,10)       << " | "
          << prd(ste.local ? "l":"g",1,10)       << " | "
          << prd(to_string(ste.id),1,10)       << endl;
    }
    return ss.str();
}
