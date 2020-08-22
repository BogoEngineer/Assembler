.extern x1
.global g1

.section .data
.skip 5
s1:
s2:
g1:
s3:.word 100, s5, g1, x1
s4:.word 0xAAAA
.skip 10
s5:
.byte 0xFF
res:
.byte 0



.section .text
mov s1, %r0

repeat:
sub s5, %r0
sub $1, %r0

cmp %r0, $1
jgt *end1(%r7)

cmp %r0, $0
jgt end0

mov $repeat, %r1
jmp *%r1

end0:
mov $0, %r0
mov $0, s4(%r0)

halt



end1:
mov $0, %r0
mov $1, s4(%r0)

halt


.section .bss
.skip 10
.end