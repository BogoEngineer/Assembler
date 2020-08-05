#ifndef TEXTMANIPULATOR_H
#define TEXTMANIPULATOR_H

#include "INCLUDES.h"
#include <vector>
#include <sstream>

class TextManipulator{
    public:
        vector<string> extractWords(string str);
        string eliminateWhiteSpace(string str);
        bool isEmpty(string str);
};

#endif