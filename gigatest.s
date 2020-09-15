#.equ a, b-5

.section prethodna
.skip 10
.word 5, a
a: jmp a(%r7)

.section main
.skip 10
#mov a(%r7), b(%r7)
jmp a(%r7)
b:

.end