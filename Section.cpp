#include "Section.h"
#include <sstream>

Section::Section(string n):name(n){
    relocation_table = {};
    machine_code = {};
    location_counter = 0;
}

string Section::getMachineCodeString(){
    return byteCodeToString(machine_code);
}

string Section::byteCodeToString(vector<char> byte_code){
    std::stringstream ss;
    for(unsigned char c: byte_code){
        if((c & 0xF0) == 0x0) ss<<"0";
        ss << std::hex << (int)c;
    }
    return ss.str();
}

vector<char>& Section::getMachineCode(){
    return machine_code;
}

/*! Center-aligns string within a field of width w. Pads with blank spaces
    to enforce alignment. */
string center1(const string s, const int w) {
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
string prd1(const string x, const int decDigits, const int width) {
    stringstream ss;
    ss << fixed << right;
    ss.fill(' ');        // fill space around displayed #
    ss.width(width);     // set  width around displayed #
    ss.precision(decDigits); // set # places after decimal
    ss << x;
    return ss.str();
}


string Section::getRelocationTable(){
    stringstream ss;
    ss  << center1("offset", 15)<< " | "
        << center1("type", 10)<< " | "
        << center1("value", 10)<<endl;
    for(RelocationTableEntry rte: relocation_table){
        ss<< prd1(to_string(rte.offset),1,15)       << " | "
          << prd1(rte.type,1,10)       << " | "
          << prd1(to_string(rte.value),1,10)<< endl;
    }
    return ss.str();
}


Section::~Section(){
    machine_code.clear();
    relocation_table.clear();
}