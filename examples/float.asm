movi r0, 65798
movi r1, 2345

movf f0, 234.2342
movf f1, 123.3425

divi r0, r1
mov r0, acc
dbr r0

mulf f0, f1
mov f0, accf
dbr f0

hlt
