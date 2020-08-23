.section .main

# with backpatching
jgt $a(%r7) # -2
mov a(%r7), %r6 # -3
mov a(%r7), b # -5

.skip 10
a: .word 0xAAAA, 15
b:

# no backpatching
jgt $a(%r7) # -2
mov a(%r7), %r6 # -3
mov a(%r7), b # -5


.end
