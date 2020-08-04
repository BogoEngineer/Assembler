#include "Assembler.h"
#include "INCLUDES.h"

Assembler::Assembler(string ifn, string ofn){
    input_file_name = ifn;
    output_file_name = ofn;

    st = new SymbolTable();
    frt = new ForwardReferenceTable();
    fm = new FileManager();
}

int Assembler::start(){
    assembly_code = fm->getContent(input_file_name);
    for(string line: assembly_code){
        processOneLine(line);
    }
    return 0;
}

string Assembler::processOneLine(string line){
    return "";
}