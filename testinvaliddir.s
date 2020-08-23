.global a, b
.extern c

.section .main
push %r2
pop %r2

symbol: .illegal directive

mov 5, %r5
push symbol
push %r2
sub $2, %r2
pop %r2

.end