#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "ForwardReferenceTable.h"
#include "SymbolTable.h"
#include "FileManager.h"

class Assembler{
    private:
        SymbolTable* st;
        ForwardReferenceTable* frt;
        FileManager* fm;
        string input_file_name;
        string output_file_name;
        vector<string> assembly_code;
        vector<string> machine_code;
        int location_counter;

        string processOneLine(string line); // one line assembly ==> one line binary
    public: 
        Assembler(string ifn, string ofn);
        int start();
};

#endif