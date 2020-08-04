#ifndef SYMBOLTABLEENTRY_H
#define SYMBOLTABLEENTRY_H
#include "INCLUDES.h"

#include "ForwardReferenceTable.h"

class SymbolTableEntry{
    public:
        string name;
        int id;
        int size;
        int value;
        int section;
        int offset;
        bool local;
        bool defined;
        ForwardReferenceTable* flink;
};

#endif