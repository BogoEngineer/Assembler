#include "Assembler.h"
#include "INCLUDES.h"

Assembler::Assembler(string ifn, string ofn){
    input_file_name = ifn;
    output_file_name = ofn;

    st = new SymbolTable();
    frt = new ForwardReferenceTable();
    fm = new FileManager();
    tm = new TextManipulator();
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
    size_t comment_start = line.find('#');
    string to_process = tm->eliminateWhiteSpace(line);
    size_t label_end = to_process.find(':');
    if(label_end != string::npos) { // label detected
        dealWithSymbol(line.substr(0, label_end));
    }
    
    string binary_code =  dealWithInstruction(line.substr(
        label_end == string::npos ? 0 : label_end + 1,
        comment_start == string::npos ? line.length() : comment_start
    )); // instruction or directive (without label) detected

    if(comment_start != string::npos) dealWithComment(line.substr(comment_start));
    return binary_code;
}

string Assembler::dealWithInstruction(string instruction){
    if(instruction[0] == '.'){
        dealWithDirective(instruction);
        return ""; // no byte code for object file
    }
    vector<string> words = tm->extractWords(instruction);
    cout<<"strictly instruction ";
    for(string word: words){
        cout<<word<<"^^^";
    }
    cout<<endl;
    return "some binary code";
}

void Assembler::dealWithDirective(string directive){
    cout<<"directive "<<directive<<endl;
}

void Assembler::dealWithSymbol(string symbol){
    cout<<"symbol "<<symbol<<endl;
}

void Assembler::dealWithComment(string comment){
    cout<<"comment "<<comment;
}