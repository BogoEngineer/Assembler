.section .text  
labela: #komentar labela  
  nesto:
    pushb $255
        pop nes_drugo
      jmp  *svasta
  labela5:
#jos jedan  
.equ prvalabela, svasta-svasta+labela5
.equ a, b
.equ b, c
.equ c, 10
    kraj: halt
ret
#komentarilic
.section .bss
 
list_byte_bss:  
#.byte 0b01110101, 0b1111110, 0x2f, 0104, 115
  src: 
labela1: .word 10
pop 20(%pc) 
movb $55, svasta
and 50(%r2), svasta(%pc)
.word 0xAA55, 12345, -1
.skip 10
svasta:

.end
