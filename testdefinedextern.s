.global a, b
.extern c

.section main
c:.byte 0x50 # illegal to define a symbol that is already declared as extern

mov $a, $x

x:
cmp $2, a
.word 25, 018, 0x26
halt 

.end
