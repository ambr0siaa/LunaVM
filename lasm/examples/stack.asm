mov r0, 1
mov r1, 2

dbr r0
dbr r1

push r0
push r1

pop r0
pop r1

dbr r0
dbr r1

mov r2, 3456879
mov f0, 123.789

sub r2, 154125
sub f0, 123.1234

dbr r2
dbr f0

push 1
hlt