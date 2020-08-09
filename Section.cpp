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

string Section::getRelocationTable(){
    return "";
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