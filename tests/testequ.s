.global a

.section some_section
.word 0x00AA

.equ a, b # unresolvable equ dependency
 .equ b,  a


 push %r3
.skip 8
jmp end

mov $5, %r2
add %r2, %r5
sub %r2, 3(%r2)
mov %r3, 5(%r2)

.section new
pop %r3
end:
.end