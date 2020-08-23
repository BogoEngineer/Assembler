#include "TextManipulator.h"
#include <algorithm>
#include <string>

vector<string> TextManipulator::extractWords(string str){
    vector<string> ret = {};
    std::stringstream ss(str);
    for (string i; ss >> i;) {
        size_t excess_comma = i.find(',');
        if(excess_comma != string::npos) i.erase(excess_comma);
        ret.push_back(i);  
        while (ss.peek() == ',' || ss.peek() == ' ')
            ss.ignore();
    }

    return ret;
}

string  TextManipulator::eliminateWhiteSpace(string str){
    size_t position_f = str.find_first_not_of(' ');
    size_t position_l = str.find_last_not_of(' ');
    return str.substr(position_f, position_l+1);
}

bool TextManipulator::isEmpty(string str){
    if(str.empty() || str.find_first_not_of(' ')==string::npos) return true;
    return false;
}