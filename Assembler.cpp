#include "Assembler.h"
#include "INCLUDES.h"
#include <algorithm>

Assembler::Assembler(string ifn, string ofn){
    input_file_name = ifn;
    output_file_name = ofn;

    st = new SymbolTable();
    frt = new ForwardReferenceTable();
    fm = new FileManager();
    tm = new TextManipulator();

    instruction_set = {
        Instruction("halt", 0x00, 0),
        Instruction("iret", 0x01, 0),
        Instruction("ret", 0x02, 0),
        Instruction("int", 0x03, 1),
        Instruction("call", 0x04, 1),
        Instruction("jmp", 0x05, 1),
        Instruction("jeq", 0x06, 1),
        Instruction("jne", 0x07, 1),
        Instruction("jgt", 0x08, 1),
        Instruction("push", 0x09, 1),
        Instruction("pop", 0x0A, 1),
        Instruction("xchg", 0x0B, 2),
        Instruction("mov", 0x0C, 2),
        Instruction("add", 0x0D, 2),
        Instruction("sub", 0x0E, 2),
        Instruction("mul", 0x0F, 2),
        Instruction("div", 0x10, 2),
        Instruction("cmp", 0x11, 2),
        Instruction("not", 0x12, 2),
        Instruction("and", 0x13, 2),
        Instruction("or", 0x14, 2),
        Instruction("xor", 0x15, 2),
        Instruction("test", 0x16, 2),
        Instruction("shl", 0x17, 2),
        Instruction("shr", 0x18, 2)
    };
}

int Assembler::start(){
    assembly_code = fm->getContent(input_file_name);
    machine_code = {};
    for(string line: assembly_code){
        machine_code.push_back(processOneLine(line));
    }
    //fm->setContent(machine_code, output_file_name);
    return 0;
}

string Assembler::processOneLine(string line){
    // Line recognition - section/instruction/label
    if(tm->isEmpty(line)) return "";
    vector<string> to_process = tm->extractWords(line);
    bool lab = false;
    if(to_process[0].find(':') != string::npos) {
        dealWithSymbol(to_process[0].substr(0, to_process[0].length()-1));
        lab = true;
    }
    bool inst = true;
    string instruction = "";
    string comment = "";
    for(string n: to_process){
        if(lab){
            lab = false;
            continue;
        } 
        if(n.find('#') != string::npos) inst = false;
        if(inst) instruction = instruction + n + " ";
        else comment = comment + n + " ";
    }
    if(instruction != "") dealWithInstruction(instruction);
    if(comment != "") dealWithComment(comment);
    return "";
}

string Assembler::dealWithInstruction(string instruction){
    if(instruction[0] == '.'){
        dealWithDirective(instruction);
        return ""; // no byte code for object file
    }
    vector<string> words = tm->extractWords(instruction);
    Instruction* inst = std::find_if(instruction_set.begin(), instruction_set.end(), find_id(words[0])).base();
    cout<<inst->name<<" "<<inst->OC;
    return "some binary code";
}

void Assembler::dealWithDirective(string directive){
}

void Assembler::dealWithSymbol(string symbol){
}

void Assembler::dealWithComment(string comment){
}