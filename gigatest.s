.equ a, b+5

.section main
.skip 10
mov b(%r7), a
b:
.end