movi r0, 1
movi r1, 2

dbr r0
dbr r1

pshr r0
pshr r1

pop r0
pop r1

dbr r0
dbr r1

movi r2, 3456879
movf f0, 123.789

subv r2, 154125
subv f0, 123.1234
mov r2, acc
mov f0, accf

dbr r2
dbr f0

push 1
hlt
