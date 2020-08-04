#ifndef FORWARDREFERENCETABLE_H
#define FORWARDREFERENCETABLE_H
#include "INCLUDES.h"
#include "ForwardReferenceTableEntry.h"
 
#include <vector>

class ForwardReferenceTable{
    private:
        vector<ForwardReferenceTableEntry> table;
};

#endif