movi r0, 1
mov r1, r0

movf f0, 1.0
movf f1, 1.0

addi r0, r1
mov r0, acc
dbr r0

addf f0, f1
mov f0, accf
dbr f0

hlt