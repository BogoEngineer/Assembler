#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "FileManager.h"
#include "TextManipulator.h"
#include "SymbolTable.h"
#include "Section.h"
#include <map>

struct IndexTableEntry{
    string section;
    int value;

    IndexTableEntry(string s, int v=0):section(s),value(v){}
};

struct find_index_table_entry : std::unary_function<IndexTableEntry, bool> {
    string section;
    find_index_table_entry(string s):section(s) { }
    bool operator()(IndexTableEntry const& i) const {
        return i.section == section;
    }
};

struct UncomputableSymbolTableEntry {
    string left_symbol;
    vector<string> needed_symbols;
    vector<IndexTableEntry> it;
    int offset;

    UncomputableSymbolTableEntry(string l, vector<IndexTableEntry> i):left_symbol(l), it(i), offset(0){
        needed_symbols = {};
    }
};

struct Instruction{
    string name;
    int OC;
    int operand_number;
    Instruction(string n, int oc, int on): name(n), OC(oc), operand_number(on){};
};

struct find_instruction : std::unary_function<Instruction, bool> {
    string name;
    find_instruction(string n):name(n) { }
    bool operator()(Instruction const& i) const {
        return i.name == name;
    }
};

struct find_section : std::unary_function<Section, bool> {
    string name;
    find_section(string n):name(n) { }
    bool operator()(Section const& sec) const {
        return sec.name == name;
    }
};

class Assembler{
    private:
        SymbolTable* st;
        FileManager* fm;
        TextManipulator* tm;
        string input_file_name;
        string output_file_name;
        vector<string> assembly_code;
        vector<Section*> sections;
        vector<Instruction> instruction_set;
        vector<UncomputableSymbolTableEntry> ust; // used for equ directives
        int line_of_code;
        Section* current_section;
        map<string, int> directive_map;

        vector<char> processOneLine(string line); // one line assembly ==> one line binary
        vector<char> dealWithInstruction(string instruction); // recognize given instruction and return binary code for given instruction
        void dealWithDirective(string directive); // recognize given directive and do stuff
        void defineSymbol(string symbol, bool local, bool defined, bool ext=false); // symbol table etc.. logic
        void dealWithComment(string comment); // probably ignore given comment, needed for testing
        SymbolTableEntry* dealWithSymbol(string symbolName, int address_field_offset); // deal with situation when symbol is found in a address field
        void dealWithSection(string section_name); // sets current section
        void dealWithRelocationRecord(string symbol, int reg_num); // will be called after dealing with a symbol inside of an instruction

        Section* findSection(string section); // finds section with given name

        string handleError(string error); // maybe create some error table and then return int as a code to take a specific message for output

        static char getAdressingMode(string operand, bool is_jump); // get addressing mode for operand
        static int determineRegister(string operand); // get register number if one is used from operand
        static char higherByteRegister(string operand); // is higher 8 or lower 8 bits used for register direct addressing mode: 0-lower, 1-higher

        int getInt(string operand);
        bool isSymbol(string x);
        map<string, int> createMap();
        void end();

        void resolveUST();

        vector<string> divideEquOperands(string expression);
    public: 
        Assembler(string ifn, string ofn);
        ~Assembler();
        int start();
};

#endif