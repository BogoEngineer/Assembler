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
    // index 0 - mnemonic, index 1 - first operand, index 2 - second operand
    Instruction* inst = std::find_if(instruction_set.begin(), instruction_set.end(), find_id(words[0])).base();
    /* 
        Structure of instruction:
        Instruction Description byte: OC4|OC3|OC2|OC1|OC0|S|Un|Un
            OC - operation code bit
            S - operand size (0 - 1 byte, 1 - 2 bytes) bit
            Un - unused bit (default 0)
        Operand Description byte: AM2|AM1|AM0|R3|R2|R1|R0|L/H
            AM - coded addressing mode:
                - Immediate: 0x0
                - Register direct: 0x1
                - Register indirect without offset: 0x2
                - Register indirect with 16 bit signed offset: 0x3
                - Memory: 0x4
            R - coded number of register used:
                0x0 - 0x7 general purpose registers (pc = 0x7, sp = 0x6)
                0xF - psw
                0xA - no register is used
            L/H - lower or higher byte is used in case of register direct addressing mode for operand with size of 1 byte
    */
   char instr_descr_byte = inst->OC<<3;
    switch(inst->operand_number){
        case 0:
            // size bit and unsused bits are 0, no need to do anything
            return "byte code";
            break;
        case 1:
        {
            bool is_jump = false;
            bool size_mask;
            if(words[0][0] == 'j' || words[0] == "int" || words[0] == "call") is_jump = true;
            char address_mode = getAdressingMode(words[1], is_jump);
            if(address_mode == 0x0 || address_mode == 0x1) size_mask = 1;
            else if(address_mode == 0x2 || address_mode == 0x3 || address_mode == 0x4) size_mask = 0;
            instr_descr_byte |= size_mask<<2;
            if(words[0] == "pop" && address_mode == 0x0) return handleError("Immediate addressing mode with destination operand is prohibited.");
            
            // op descr byte
            char op_descr_byte = address_mode << 5;
            char register_bits = 0xA;
            if(address_mode == 0x1 || address_mode ==  0x2 || address_mode == 0x3){
                register_bits = determineRegister(words[1]);
            }

            op_descr_byte |= (register_bits<<1);

            // L/H bit
            if(address_mode == 0x1 && size_mask == 0){
                op_descr_byte |= higherByteRegister(words[1]);
            }

            // operand size bytes
            if(address_mode == 0x1 || address_mode == 0x2) return "byte code";    
            
            char operand1_related_byte1;
            char operand1_related_byte2;
            if(address_mode == 0x3 || address_mode == 0x4){
                // filling those up from symbol table etc...
                return "byte code";
            }

            if(address_mode == 0x0){
                short literal = stoi(words[1].find('$') == string::npos ? words[1] : words[1].substr(1));
                if(size_mask == 0){ 
                    operand1_related_byte1 = literal;
                    return "byte code";
                }else{
                    // Assumption that  higher byte goes to first and lower to second bytes in instruction encoding
                    operand1_related_byte1 = literal & 0xFF00; 
                    operand1_related_byte2 = literal & 0x00FF;
                    return "byte code";
                }
            }
            break;
        }
        case 2:
        {
            bool size_mask;
            char address_mode1 = getAdressingMode(words[1], false);
            char address_mode2 = getAdressingMode(words[2], false);
            if(address_mode2 == 0x0 && (words[0] != "cmp" && words[0] != "test")) return handleError("Immediate addressing mode with destination operand is prohibited.");
            char addres_mode = (address_mode1 > address_mode2) ? address_mode1 : address_mode2;
            if(address_mode1 == 0x0 || address_mode1 == 0x1) size_mask = 1;
            else if(address_mode1 == 0x2 || address_mode1 == 0x3 || address_mode1 == 0x4) size_mask = 0;
            instr_descr_byte |= size_mask<<2;

            // operand bytes

            // FIRST OPERAND
            char op_descr_byte1 = address_mode1 << 5;
            char register_bits1 = 0xA;

            if(address_mode1 == 0x1 || address_mode1 ==  0x2 || address_mode1 == 0x3){
                register_bits1 = determineRegister(words[1]);
            }

            op_descr_byte1 |= (register_bits1<<1);

            // L/H bit
            if(address_mode1 == 0x1 && size_mask == 0){
                op_descr_byte1 |= higherByteRegister(words[1]);
            }

            if(address_mode1 == 0x1 || address_mode1 == 0x2){};    
            
            char operand1_related_byte1;
            char operand1_related_byte2;
            if(address_mode1 == 0x3 || address_mode1 == 0x4){
                // filling those up from symbol table etc...
            }

            if(address_mode1 == 0x0){
                short literal = stoi(words[1].find('$') == string::npos ? words[1] : words[1].substr(1));
                if(size_mask == 0){ 
                    operand1_related_byte1 = literal;
                }else{
                    // Assumption that  higher byte goes to first and lower to second bytes in instruction encoding
                    operand1_related_byte1 = literal & 0xFF00; 
                    operand1_related_byte2 = literal & 0x00FF;
                }
            }

            // SECOND OPERAND
            char op_descr_byte2 = address_mode2 << 5;
            char register_bits2 = 0xA;

            if(address_mode2 == 0x1 || address_mode2 ==  0x2 || address_mode2 == 0x3){
                register_bits2 = determineRegister(words[2]);
            }

            op_descr_byte2 |= (register_bits2<<1);

            // L/H bit
            if(address_mode2 == 0x1 && size_mask == 0){
                op_descr_byte2 |= higherByteRegister(words[2]);
            }

            if(address_mode2 == 0x1 || address_mode2 == 0x2) return "byte code";    
            
            char operand2_related_byte1;
            char operand2_related_byte2;
            if(address_mode2 == 0x3 || address_mode2 == 0x4){
                // filling those up from symbol table etc...
                return "byte code";
            }

            if(address_mode2 == 0x0){
                short literal = stoi(words[2].find('$') == string::npos ? words[2] : words[2].substr(1));
                if(size_mask == 0){ 
                    operand2_related_byte1 = literal;
                    return "byte code";
                }else{
                    // Assumption that  higher byte goes to first and lower to second bytes in instruction encoding
                    operand2_related_byte1 = literal & 0xFF00; 
                    operand2_related_byte2 = literal & 0x00FF;
                    return "byte code";
                }
            }
        }
    }
    return "some binary code";
}

void Assembler::dealWithDirective(string directive){
}

void Assembler::dealWithSymbol(string symbol){
}

void Assembler::dealWithComment(string comment){
}

char Assembler::getAdressingMode(string operand, bool is_jump){
    if(is_jump){
        if(operand[0]=='*'){
            switch(operand[1]){
                case '%':
                    return 0x1;
                    break;
                case '(':
                    return 0x2;
                    break;
                default:
                    if(operand.find('(') != string::npos) return 0x3;
                    return 0x4;
            }
        }
        else{
            return 0x0;
        }
    }else{
        switch(operand[0]){
            case '$': return 0x0;
                break;
            case '%': return 0x1;
                break;
            case '(': return 0x2;
                break;
            default:
                if(operand.find('(') != string::npos) return 0x3;
                return 0x4;
        }
    }
}   

char Assembler::determineRegister(string operand){
    int start = operand.find('%');
    if(start == string::npos) {
        cout<<"ERROR: REGISTER COULDNT BE FOUND IN OPERAND "<<operand<<endl;
        return 16;
    }
    string rgstr = operand.substr(start+1, 2);
    if(rgstr[0] != 'r') {
        cout<<"ERROR: REGISTER COULDNT BE FOUND IN OPERAND "<<operand<<endl;
        return 16;
    }
    return rgstr[1];
}

char Assembler::higherByteRegister(string operand){
    int start = operand.find('%');
    if(start == string::npos) {
        cout<<"ERROR: REGISTER COULDNT BE FOUND IN OPERAND "<<operand<<endl;
        return 16;
    }
    string rgstr = operand.substr(start+1, 3);
    return rgstr[2] == 'h' ? 1 : 0;
}

string Assembler::handleError(string error){
    cout<<error<<endl;
    return error;
}