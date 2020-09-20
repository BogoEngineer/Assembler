.global a,b
.extern c

.section text
.equ d, 0x10-a+sim3
jmp *sim2(%r5)
sim1: .byte 0B00011
movw  $0x1A, sim1
.equ x, a
b: 
push $a

.section data
sim2: jgt *12(%r2)
jne sim1
.word 0x7,c,sim3
and (%r3), sim2
shr %r0, $sim2

.section sekcija_3
test sim3(%pc)  ,   %r1
a: mov %r4, 0b0110(%r6)
jeq *sim1(%r7)
sim3: subb $2, %r5l
pop %r6

.end