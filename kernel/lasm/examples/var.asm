summ:
    add r0, r1
    mov r0, acc
    dbr r0
    ret

.entry main:
    mov r0, 12
    mov r1, 34
    push 123.56

    dbr f0
    dbr r0
    dbr r1

    call summ

    hlt