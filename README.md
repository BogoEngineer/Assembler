# SystemSoftware

## Quick Introduction
A one pass assembler for the processor described in the section processor. 

## Processor

###### General
16-bit processor with Von-Neuman architecture and two address instruction format. 

###### Memory
Memory is byte-addressible with little endian storage system. Stack grows by decreasing memory address.

###### Registers
The processor also possesses eight general-purpose registers with names r\<num>, where \<num> corresponds to a specific register and is in [0, 7] range. 
Registers r6 and r7 are being used exclusively as pc (program counter) and sp (stack pointer) registers, respectively. Apart from general purpose registers, there is a psw (program status word) register.
  
###### Instructions
The size of an instruction varies from one to seven bytes. Generally speaking, it has a following format: 

|InstrDescr|Op1Descr|Im/Di/Ad|Im/Di/Ad|Op2Descr|Im/Di/Ad|Im/Di/Ad|
| -------- | ------ | ------ | ------ | ------ | ------ | ------ |

First byte represents OP code with some additional info about coded instruction. Other bytes are used as codes for instruction operand(s). InstrDescr and Op<num>Descr bytes have following formats:

|OC4|OC3|OC2|OC1|OC0|S|Un|Un|
| - | - | - | - | - |-|--|--|

Meaning of individual bits:
- OC4 to OC0 represent operation code of an instruction
- S(Size) represents operand size (0 means 1, and 1 means 2 bytes)
- Un(Unused) are unused bits that have default value of 0

|AM2|AM1|AM0|R3|R2|R1|R0|L/H|
| - | - | - |--|--|--|--|---|

Meaning of individual bits:
- AM2 to AM0 represent code for a addressing mode:
  - 0x0 - immediate 
  - 0x1 - register direct
  - 0x2 - register indirect without offset
  - 0x3 - register idirect with offset
  - 0x4 - memory
- R3 to R0 represent code for a number of register that is used (if none, code is 0xA)
- L/H tells wether are higher or lower 8bits from the register used 

Instruction set:
|Mnemonic|OC|Effect|Affected flags|
| --- | --- | --- | --- |
|halt|0| End of instruction computing|-|
|iret|1|pop psw; pop pc;|psw|
|ret|2|pop|pc;|-|
|int dst|3|push psw; pc<=mem16[(dst mod 8)*2];|-|
call dst|4|push|pc;|pc<=dst;|-
jmp dst|5|pc<=dst;|-
jeq dst|6|if (equal) pc<=dst;|-
jne dst|7|if (not_equal) pc<=dst;|-
jgt dst|8|if (signed_greater) pc<=dst;|-
push src|9|sp<=sp-2; mem16[sp]<=src;|-
pop dst|10|dst<=mem16[sp]; sp<=sp+2;|-
xchg src, dst|11|temp<=dst; dst<=src; src<=temp;|-
mov src, dst|12|dst<=src;|Z N
add src, dst|13|dst<=dst+src;|Z O C N
sub src, dst|14|dst<=dst-src;|Z O C N
mul src, dst|15|dst<=dst*src;|Z N
div src, dst|16|dst<=dst/src;|Z N
cmp src, dst|17|temp<=dst-src;|Z O C N
not src, dst|18|dst<=~src;|Z N
and src, dst|19|dst<=dst&src;|Z N
or src, dst|20|dst<=dst\|src;|Z N
xor src, dst|21|dst<=dst^src;|Z N
test src, dst|22|temp<=dst&src;|Z N
shl src, dst|23|dst<=dst<<src;|Z C N
shr dst, src|24|dst<=dst>>src;|Z C N

Operand syntax in instructions that have access to data:
- $\<literal> - immediate value of \<literal>
- $\<symbol> - immediate value of \<symbol>
- %r\<num> - value form register r<num>
- \(%r<num>) - value from memory on address that is value of r\<num>
- \<literal>(r\<num>) - value from memory on address \<literal> + r\<num>
- \<symbol>(r\<num>) - value from memory on address \<symbol> + r\<num>
- \<symbol>(r7/pc) - value from memory on address \<symbol> (PC relative)
- \<literal> - value from memory on address \<literal> (absolute)
- \<symbol> - value from memory on address \<symbol> (absolute)
  
Operand syntax in flow-control instructions:
- \<literal> - jump to an address \<literal>
- \<symbol> - jump to an address \<symbol>
- *%r\<num> - jump to an address r<num>
- *\(%r<num>) - jump to an address from memory on address that is value of r\<num>
- *\<literal>(r\<num>) - jump to an address from memory on address \<literal> + r\<num>
- *\<symbol>(r\<num>) - jump to an address from memory on address \<symbol> + r\<num>
- *\<symbol>(r7/pc) - jump to an address from memory on address \<symbol> (PC relative)
- *\<literal> - jump to an address from memory on address \<literal> (absolute)
- *\<symbol> - jump to an address from memory on address \<symbol> (absolute)
  
###### Directives
Following directives are supported:
- .global \<symbol_list>
- .extern \<symbol_list>
- .section \<section_name>
- .end
- .byte \<symbol_list/literal_list>
- .word \<symbol_list/literal_list>
- .skip \<literal>
- .equ \<symbol>, \<expr>

Individual functionality corresponds to a matcing directive from GNU assembler documentation.

## Additional
Rules that were/should be followed when writing assembly code:
- one line of source code contains at most one instruction/directive
- label (that ends with colon) can be found only on the beggining of the source code line, and can stand alone in that line
- char b/w at the end of instruction mnemonic in source code (if none is provided, default is w) determines size of an operand for given instruction

Tests (source code) are provided in folder tests. 
The output of the assembler is an object file that contains:
- Relocation table for every section specified in source code
- Symbol table
- Machine code for every section specified in source code
