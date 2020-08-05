#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "ForwardReferenceTable.h"
#include "FileManager.h"
#include "TextManipulator.h"
#include "SymbolTable.h"

struct Instruction{
    string name;
    int OC;
    int operand_number;
    Instruction(string n, int oc, int on): name(n), OC(oc), operand_number(on){};
};

struct find_id : std::unary_function<Instruction, bool> {
    string name;
    find_id(string n):name(n) { }
    bool operator()(Instruction const& i) const {
        return i.name == name;
    }
};

class Assembler{
    private:
        SymbolTable* st;
        ForwardReferenceTable* frt;
        FileManager* fm;
        TextManipulator* tm;
        string input_file_name;
        string output_file_name;
        vector<string> assembly_code;
        vector<string> machine_code;
        vector<Instruction> instruction_set;
        int location_counter;

        string processOneLine(string line); // one line assembly ==> one line binary
        string dealWithInstruction(string instruction); // recognize given instruction and return binary code for given instruction
        void dealWithDirective(string directive); // recognize given directive and do stuff
        void dealWithSymbol(string symbol); // symbol table etc.. logic
        void dealWithComment(string comment); // probably ignore given comment, needed for testing

    public: 
        Assembler(string ifn, string ofn);
        int start();
};

#endif