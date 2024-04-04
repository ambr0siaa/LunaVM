mov r0, 65798
mov r1, 2345

mov f0, 234.2342
mov f1, 123.3425

div r0, r1
mov r0, acc
dbr r0

mul f0, f1
mov f0, accf
dbr f0

hlt