
.global a

.section neka_sekcija
.word 0x00AA

.equ a, +1-end+6

 push %r3
.skip 8
jmp end

mov $5, %r2
add %r2, %r5
sub %r2, $3 # invalid addressing mode
mov %r3, 5(%r2)

.section nova
pop %r3
end:
.end