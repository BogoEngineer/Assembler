#ifndef FORWARDREFERENCETABLE_H
#define FORWARDREFERENCETABLE_H
#include "INCLUDES.h" 
#include <vector>

struct ForwardReferenceTableEntry{
    int patch;
    int byte;
};

class ForwardReferenceTable{
    private:
        vector<ForwardReferenceTableEntry> table;
};

#endif