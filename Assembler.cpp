#include "Assembler.h"
#include "INCLUDES.h"
#include <algorithm>
#include <map>
#include <math.h>

Assembler::Assembler(string ifn, string ofn){
    input_file_name = ifn;
    output_file_name = ofn;

    line_of_code = 0;

    st = new SymbolTable();
    fm = new FileManager();
    tm = new TextManipulator();
    current_section = nullptr;
    
    sections = {};

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

    directive_map = createMap();
}

int Assembler::start(){
    assembly_code = fm->getContent(input_file_name);
    for(string line: assembly_code){
        
        vector<char> processedLine = processOneLine(line);
        //machine_code.push_back(processedLine);

        current_section->getMachineCode().insert(current_section->machine_code.end(), processedLine.begin(), processedLine.end());
    }
    return 0;
}

vector<char> Assembler::processOneLine(string line){
    line_of_code += 1;
    // Line recognition - section/instruction/label
    if(tm->isEmpty(line)) return {};
    vector<string> to_process = tm->extractWords(line);
    bool lab = false;
    if(to_process[0].find(':') != string::npos) {
        if(current_section == nullptr) handleError("Can't have label outside of a section.");
        defineSymbol(to_process[0].substr(0, to_process[0].length()-1), true, true);
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
    if(instruction != "") return dealWithInstruction(instruction);
    if(comment != "") dealWithComment(comment);
    return {};
}

vector<char> Assembler::dealWithInstruction(string instruction){
    if(instruction[0] == '.'){
        dealWithDirective(instruction.substr(1));
        return {}; // no byte code for object file
    }
    if(current_section == nullptr) handleError("Can't have instruction outside of a section.");
    vector<string> words = tm->extractWords(instruction); 
    //cout<<"INSTRUCTION: "<< words[0]<<endl;
    // index 0 - mnemonic, index 1 - first operand, index 2 - second operand
    Instruction* inst = std::find_if(instruction_set.begin(), instruction_set.end(), find_instruction(words[0])).base();
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
    char instr_descr_byte = (inst->OC)<<3;
    vector<char> byte_code = {instr_descr_byte}; // array of bytes for object file ... up to 7 per instruction
    switch(inst->operand_number){
        case 0:{
            // size bit and unsused bits are 0, no need to do anything
            
            current_section->location_counter += byte_code.size();
            return byte_code;
            break;
        }
        case 1: {
            bool is_jump = false;
            bool size_mask;
            if(words[0][0] == 'j' || words[0] == "int" || words[0] == "call") is_jump = true;
            char address_mode = getAdressingMode(words[1], is_jump);
            if(address_mode == 0x0 || address_mode == 0x1) size_mask = 1;
            else if(address_mode == 0x2 || address_mode == 0x3 || address_mode == 0x4) size_mask = 0;
            instr_descr_byte |= size_mask<<2;
            if(words[0] == "pop" && address_mode == 0x0) handleError("Immediate addressing mode with destination operand is prohibited.");
            
            // op descr byte
            unsigned char op_descr_byte = address_mode << 5;
            
            unsigned char register_bits = 0xA;
            if(address_mode == 0x1 || address_mode ==  0x2 || address_mode == 0x3){
                register_bits = determineRegister(words[1]);
            }

            op_descr_byte |= (register_bits<<1);
            //cout<<" Address mode: "<<(int)op_descr_byte;
            // L/H bit
            if(address_mode == 0x1 && size_mask == 0){
                op_descr_byte |= higherByteRegister(words[1]);
            }

            byte_code.push_back(op_descr_byte);

            
            if(address_mode == 0x1 || address_mode == 0x2){
                current_section->location_counter += byte_code.size();
                return byte_code; // there are no operand related bytes
            } 
            

            // operand size bytes
            unsigned char operand1_related_byte1;
            unsigned char operand1_related_byte2;
            if(address_mode == 0x3 || address_mode == 0x4){
                size_t potential_reg_ind = words[1].find('(');
                int register_num = 10;
                string potential_symbol = words[1];
                if(potential_reg_ind != string::npos) {
                    register_num = determineRegister(words[1]);
                    potential_symbol = potential_symbol.substr(0, potential_reg_ind);
                }
                if(!isSymbol(potential_symbol)){
                    short int literal = stoi(potential_symbol);
                    operand1_related_byte1 = (literal>>8) & 0xFF;
                    operand1_related_byte2 = literal & 0xFF;
                }else{
                    string symbol_name = potential_symbol;
                    if(symbol_name[0] == '*' || symbol_name[0] == '$') symbol_name = symbol_name.substr(1);
                    SymbolTableEntry* ste = dealWithSymbol(symbol_name, 2);
                    if(ste->defined == true){
                        operand1_related_byte1 = (ste->offset>>8) & 0xFF;
                        operand1_related_byte2 = (ste->offset) & 0xFF;
                    }else{
                        operand1_related_byte1 = 0;
                        operand1_related_byte2 = 0;
                    } 
                    dealWithRelocationRecord(symbol_name, register_num);
                }

                byte_code.push_back(operand1_related_byte1);
                byte_code.push_back(operand1_related_byte2);

                current_section->location_counter += byte_code.size();
                return byte_code;
            }

            if(address_mode == 0x0){
                short literal = stoi(words[1].find('$') == string::npos ? words[1] : words[1].substr(1));
                if(size_mask == 0){ 
                    operand1_related_byte1 = literal;
                    byte_code.push_back(operand1_related_byte1);

                    current_section->location_counter += byte_code.size();
                    return byte_code;
                }else{
                    // Assumption that  higher byte goes to first and lower to second bytes in instruction encoding
                    operand1_related_byte1 = (literal>>8) & 0xFF; 
                    operand1_related_byte2 = literal & 0xFF;

                    byte_code.push_back(operand1_related_byte1);
                    byte_code.push_back(operand1_related_byte2);

                    current_section->location_counter += byte_code.size();
                    return byte_code;
                }
            }
            break;
        }
        case 2:{
            bool size_mask;
            char address_mode1 = getAdressingMode(words[1], false);
            char address_mode2 = getAdressingMode(words[2], false);
            if(address_mode2 == 0x0 && (words[0] != "cmp" && words[0] != "test")) handleError("Immediate addressing mode with destination operand is prohibited.");
            char addres_mode = (address_mode1 > address_mode2) ? address_mode1 : address_mode2;
            if(address_mode1 == 0x0 || address_mode1 == 0x1) size_mask = 1;
            else if(address_mode1 == 0x2 || address_mode1 == 0x3 || address_mode1 == 0x4) size_mask = 0;
            instr_descr_byte |= size_mask<<2;

            // operand bytes

            int address_field_offset = 1;

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

            address_field_offset += 1;
            byte_code.push_back(op_descr_byte1);

            if(address_mode1 == 0x1 || address_mode1 == 0x2){};    // no pushing operand related bytes
            
            char operand1_related_byte1;
            char operand1_related_byte2;
            if(address_mode1 == 0x3 || address_mode1 == 0x4){
                size_t potential_reg_ind = words[1].find('(');
                int register_num = 10;
                string potential_symbol = words[1];
                if(potential_reg_ind != string::npos) {
                    register_num = determineRegister(words[1]);
                    potential_symbol = potential_symbol.substr(0, potential_reg_ind);
                }
                if(!isSymbol(potential_symbol)){
                    short int literal = stoi(potential_symbol);
                    operand1_related_byte1 = (literal>>8) & 0xFF;
                    operand1_related_byte2 = literal & 0xFF;
                }else{
                    string symbol_name = potential_symbol;
                    if(symbol_name[0] == '*' || symbol_name[0] == '$') symbol_name = symbol_name.substr(1);
                    SymbolTableEntry* ste = dealWithSymbol(symbol_name, 2);
                    if(ste->defined == true){
                        operand1_related_byte1 = (ste->offset>>8) & 0xFF;
                        operand1_related_byte2 = ste->offset & 0xFF;
                    }else{
                        operand1_related_byte1 = 0;
                        operand1_related_byte2 = 0;
                    }

                    dealWithRelocationRecord(symbol_name, register_num);
                }

                address_field_offset += 2;
                byte_code.push_back(operand1_related_byte1);
                byte_code.push_back(operand1_related_byte2);
            }

            if(address_mode1 == 0x0){
                short literal = stoi(words[1].find('$') == string::npos ? words[1] : words[1].substr(1));
                if(size_mask == 0){ 
                    operand1_related_byte1 = literal;
                    address_field_offset += 1;
                    byte_code.push_back(operand1_related_byte1);
                }else{
                    // Assumption that  higher byte goes to first and lower to second bytes in instruction encoding
                    operand1_related_byte1 = (literal>>8) & 0xFF; 
                    operand1_related_byte2 = literal & 0xFF;

                    address_field_offset += 2;
                    byte_code.push_back(operand1_related_byte1);
                    byte_code.push_back(operand1_related_byte2);
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

            address_field_offset += 1;
            byte_code.push_back(op_descr_byte2);

            if(address_mode2 == 0x1 || address_mode2 == 0x2) {
                current_section->location_counter += byte_code.size();
                return byte_code;    
            }

            char operand2_related_byte1;
            char operand2_related_byte2;
            if(address_mode2 == 0x3 || address_mode2 == 0x4){
                size_t potential_reg_ind = words[2].find('(');
                int register_num = 10;
                string potential_symbol = words[2];
                if(potential_reg_ind != string::npos) {
                    register_num = determineRegister(words[2]);
                    potential_symbol = potential_symbol.substr(0, potential_reg_ind);
                }

                if(!isSymbol(potential_symbol)){
                    short int literal = stoi(potential_symbol);
                    operand1_related_byte1 = (literal>>8) & 0xFF;
                    operand1_related_byte2 = literal & 0xFF;
                }else{
                    string symbol_name = potential_symbol;
                    if(symbol_name[0] == '*' || symbol_name[0] == '$') symbol_name = symbol_name.substr(1);
                    SymbolTableEntry* ste = dealWithSymbol(symbol_name, address_field_offset);
                    if(ste->defined == true){
                        operand2_related_byte1 = (ste->offset>>8) & 0xFF;
                        operand2_related_byte2 = ste->offset & 0xFF;
                    }else{
                        operand2_related_byte1 = 0;
                        operand2_related_byte2 = 0;
                    }

                    dealWithRelocationRecord(symbol_name, register_num);
                }
                byte_code.push_back(operand2_related_byte1);
                byte_code.push_back(operand2_related_byte2);

                current_section->location_counter += byte_code.size();
                return byte_code;
            }

            if(address_mode2 == 0x0){
                short literal = stoi(words[2].find('$') == string::npos ? words[2] : words[2].substr(1));
                if(size_mask == 0){ 
                    operand2_related_byte1 = literal;
                    byte_code.push_back(operand2_related_byte1);

                    current_section->location_counter += byte_code.size();
                    return byte_code;
                }else{
                    // Assumption that  higher byte goes to first and lower to second bytes in instruction encoding
                    operand2_related_byte1 = literal & 0xFF00; 
                    operand2_related_byte2 = literal & 0x00FF;
                    byte_code.push_back(operand2_related_byte1);
                    byte_code.push_back(operand2_related_byte2);

                    current_section->location_counter += byte_code.size();
                    return byte_code;
                }
            }
        }
    }
    handleError("Too much or too many operands for instruction!");
    return {};
}

void Assembler::dealWithDirective(string directive){
    vector<string> words = tm->extractWords(directive); 
    if(directive_map.find(words[0]) == directive_map.end()) handleError("Directive does not exist.");
    switch (directive_map[words[0]])
    {
    case 0: // .section
        dealWithSection(words[1]);
        break;
    case 1: // .equ
    {   
        string symbol_name = words[1];
        vector<IndexTableEntry> index_table = {};
        UncomputableSymbolTableEntry* uste = new UncomputableSymbolTableEntry(symbol_name, index_table);
        int offset = 0;
        int sign=1; // 1 --> + | -1 --> -
        bool last; // true --> symbol | false --> operator
        vector<string> divided = divideEquOperands(words[2]);
        for(int i = 0; i<divided.size(); i++){
            if(divided[i] == "+"){
                sign=1;
                last = false;
                continue;
            }
            else if(divided[i] == "-"){
                sign = -1;
                last = false;
                continue;
            }
            else {
                last = true;
                if(isSymbol(divided[i])){
                    SymbolTableEntry* found = st->findSymbol(divided[i]);
                    if(found == nullptr) {
                        st->addSymbol(*(new SymbolTableEntry(divided[i])));
                        uste->needed_symbols.push_back(to_string(sign) + divided[i]);
                        continue;
                    }
                    if(found->defined == false){
                        uste->needed_symbols.push_back(to_string(sign) + divided[i]);
                    }
                    else{ 
                        offset += sign*found->offset;
                        vector<IndexTableEntry>::iterator it = std::find_if(index_table.begin(), index_table.end(), find_index_table_entry(found->section));
                        if(it != index_table.end()){
                            index_table.push_back(*(new IndexTableEntry(found->section, sign)));
                        }
                        else{
                            it.base()->value += sign;
                        }
                    }
                }else{
                    offset += getInt(divided[i]);
                }
            }
        }
        if(uste->needed_symbols.size() > 0){
                uste->offset = offset;
                ust.push_back(*uste);
                st->addSymbol(*(new SymbolTableEntry(symbol_name, current_section->name, 0, true, false)));
        }else{
            int num = 0;
            for(IndexTableEntry ite: index_table){
                if(ite.value != 0 && ite.value != 1) handleError("Illegal expression.");
                if(ite.value == 1 && num == 0) num = 1;
                if(ite.value ==1 && num == 1) handleError("Illegal expression.");
            }
            st->addSymbol(*(new SymbolTableEntry(symbol_name, current_section->name, offset, true, true)));
        }
        break;
    }
    case 2: // .end
    {
        end();
        exit(0);
        break;
    }
    case 3: // .global
        defineSymbol(words[1], false, false);
        break;
    case 4: // .extern
        defineSymbol(words[1], false, false, true);
        break;
    case 5: // .byte
        char byte;
        for(int i=1; i<words.size(); i++){
            byte = (char)getInt(words[i]);
            current_section->getMachineCode().push_back(byte);
        }
        break;
    case 6: // .word
        short int word;
        for(int i=1; i<words.size(); i++){
            word = (short int)getInt(words[i]);
            current_section->getMachineCode().push_back(word&0xFF);
            current_section->getMachineCode().push_back((word>>8)&0xFF); // little endian
        }
        break;
    case 7: // .skip
        int num_of_bytes = getInt(words[1]);
        for(int i = 0; i < num_of_bytes; i++){
            current_section->getMachineCode().push_back(0);
        }
        break;
    }
}

int Assembler::getInt(string operand){
    int ret=0;
    if(operand[0]== '\''){
        return operand[1];
    }
    else if(operand[0]=='0'){
        if(operand[1] == 'b' || operand[1]=='B'){
            string binary = operand.substr(2);
            reverse(binary.begin(), binary.end());
            for(int i=0; i<binary.size(); i++){
                ret += (binary[i]-'0')*pow(2,i);
            }
        }
        else if(operand[1]=='x' || operand[1]=='X'){
            string hex = operand.substr(2);
            return stoi(hex,0,16);
        }
        else {
            string oct = operand.substr(1);
            reverse(oct.begin(), oct.end());
            for(int i=0; i<oct.size(); i++){
                ret += (oct[i]-'0')*pow(8,i);
            }
        }
        return ret;
    }
    else{
        return stoi(operand);
    }
}

void Assembler::defineSymbol(string symbol, bool local, bool defined, bool ext){
    SymbolTableEntry* found = st->findSymbol(symbol);
    if(found == nullptr) st->addSymbol
    (*(new SymbolTableEntry(symbol, ext ? "UND" : current_section->name.substr(1), ext ? 0 : current_section->location_counter, local, defined)));
    else
    {
        if(ext) handleError("Symbol is already declared as extern.");
        if(found->defined == true) handleError("Symbol cant be defined more than once: " + symbol);
        found->defined = true;
        found->offset = current_section->location_counter;
        found->section = current_section->name.substr(1);
    }
}

void Assembler::dealWithComment(string comment){
}

void Assembler::dealWithRelocationRecord(string symbol, int register_num){
    string type = "R_386_16";
    if(register_num == 7) type = "R_386_PC16";
    int symb_id = st->findSymbol(symbol)->id;
    current_section->relocation_table.push_back(*(new RelocationTableEntry(current_section->location_counter, symb_id, type)));
}

char Assembler::getAdressingMode(string operand, bool is_jump){
    if(is_jump){
        if(operand[0] == '*'){
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

int Assembler::determineRegister(string operand){
    int start = operand.find('%');
    if(start == string::npos) {
        //cout<<"ERROR: REGISTER COULDNT BE FOUND IN OPERAND "<<operand<<endl;
        return 10;
    }
    string rgstr = operand.substr(start+1, 2);
    if(rgstr == "pc") return 7;
    if(rgstr[0] != 'r') {
        //cout<<"ERROR: REGISTER COULDNT BE FOUND IN OPERAND "<<operand<<endl;
        return 10;
    }
    return rgstr[1]-'0';
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

SymbolTableEntry* Assembler::dealWithSymbol(string symbolName, int address_field_offset){
    SymbolTableEntry* found = st->findSymbol(symbolName);
    if(found == nullptr){
        st->addSymbol(*(new SymbolTableEntry(symbolName, current_section->name.substr(1))));
        SymbolTableEntry* added = st->findSymbol(symbolName);
        added->addForwardReference(*(new ForwardReferenceTableEntry(current_section->location_counter + address_field_offset, current_section->name.substr(1))));
        return added;
    }else{
        if(found->defined == true){
            return found;
        }else{
            found->addForwardReference(*(new ForwardReferenceTableEntry(current_section->location_counter + address_field_offset, current_section->name.substr(1))));
            return found;
        }
    }
}

string Assembler::handleError(string error){
    cout<<error<<endl;
    abort();
}

void Assembler::dealWithSection(string section_name){
    Section* found = findSection(section_name);
    if(found != nullptr){
        current_section = found;
        return;
    }
    current_section = new Section(section_name);
    sections.push_back(current_section);
    st->addSymbol(*(new SymbolTableEntry(section_name, section_name.substr(1), 0, true, true)));
    return;
}

Section* Assembler::findSection(string section_name){
    for(Section* section: sections){
        if(section->name == section_name) return section;
    }
    return nullptr;
}

map<string,int> Assembler::createMap()
{
  map<string,int> m;
  m["section"] = 0;
  m["equ"] = 1;
  m["end"] = 2;
  m["global"] = 3;
  m["extern"] = 4;
  m["byte"] = 5;
  m["word"] = 6;
  m["skip"] = 7;
  return m;
}

bool Assembler::isSymbol(string x){
    return !all_of(x.begin(), x.end(), ::isdigit); // differentiating symbols and literals
}

void Assembler::end(){
    //resolveUST();
    for(Section* section: sections){
        st->backpatch(section->machine_code, section->name);
        cout<<"#.ret"<<section->name<<endl;
        cout<<section->getRelocationTable()<<endl;
    }

    cout<<st->toString();


    cout<<"MACHINE CODE:"<<endl;
    for(Section* s: sections){
        cout<<"#"<<s->name<<endl;
        cout<<s->getMachineCodeString()<<endl;
    }
    
    //fm->setContent(machine_code, output_file_name);
}

vector<string> Assembler::divideEquOperands(string expression){
    vector<string> ret = {};
    if(expression[0]=='+' || expression[0]=='-'){
        ret.push_back(string(1,expression[0]));
        expression = expression.substr(1);
    }
    size_t next_operator_plus;
    size_t next_operator_minus;
    size_t next_operator;
    while(1){
        next_operator_plus = expression.find('+');
        next_operator_minus = expression.find('-');
        if(next_operator_plus == string::npos && next_operator_minus == string::npos){
            next_operator = string::npos;
            ret.push_back(expression);
            break;
        }
        else if(next_operator_plus == string::npos && next_operator_minus != string::npos){
            next_operator = next_operator_minus;
        }
        else if(next_operator_plus != string::npos && next_operator_minus == string::npos){
            next_operator = next_operator_plus;
        }
        else{
            next_operator = next_operator_minus > next_operator_plus ? next_operator_plus : next_operator_minus;
        }
        ret.push_back(expression.substr(0, next_operator));
        ret.push_back(string(1, expression[next_operator]));
        expression = expression.substr(next_operator+1);
    }
    return ret;
}

Assembler::~Assembler(){
    delete st;
    delete fm;
    delete tm;
    sections.clear();
    instruction_set.clear();
    delete current_section;
    directive_map.clear();
}

void Assembler::resolveUST(){
    int last = ust.size();
    //cout<<"LAST: "<<last<<endl;
    while(ust.size()>0){
        for(vector<UncomputableSymbolTableEntry>:: iterator iter = ust.begin(); iter != ust.end(); ++iter){
            UncomputableSymbolTableEntry uste = *iter;
            bool valid = true;
            int offset = uste.offset;
            vector<IndexTableEntry> index_table = uste.it;
            for(vector<string>:: iterator itr = uste.needed_symbols.begin(); itr != uste.needed_symbols.end(); ++itr){
                string symbol = *itr;
                cout<<"NEEDED SYMBOL: "<<symbol<<endl;
                int sign = symbol[0]-'0';
                string symb = (sign == 1 ? symbol.substr(1) : symbol.substr(2));
                SymbolTableEntry* found = st->findSymbol(symb);
                if(found->defined == false) continue;
                //uste.needed_symbols.erase(itr);
                offset += sign*found->offset;
                string section = found->section;
                vector<IndexTableEntry>::iterator it = std::find_if(index_table.begin(), index_table.end(), find_index_table_entry(section));
                if(it != index_table.end()){
                    it.base()->value += sign;
                }    
                else{
                    index_table.push_back(*(new IndexTableEntry(found->section, sign)));
                }
            }
            if(uste.needed_symbols.size() == 0){
                int num = 0;
                for(IndexTableEntry ite: index_table){
                    if(ite.value != 0 && ite.value != 1) handleError("Illegal expression.");
                    if(ite.value == 1 && num == 0) num = 1;
                    if(ite.value ==1 && num == 1) handleError("Illegal expression.");
                }
                SymbolTableEntry* left = st->findSymbol(uste.left_symbol);
                left->defined = true;
                left->offset = offset;
                ust.erase(iter);
            }
        }
        if(last == ust.size()) handleError("Cannot resolve .equ dependencies.");
        last = ust.size();
    }
    //cout<<"ZAVRSIO I OVO"<<endl;
}