.extern x1,x2,x3
.global s3

.section .data
s1:.byte -1
.byte 100
s2:.word s1
s3:.byte 0

.section .text
push %r0

pop $100
add %r0, $100


pop %r0
halt

.section .bss
.skip 10

.end