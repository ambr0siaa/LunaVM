mov r0, 1
mov r1, 1

main:
    cmp r1, r2
    jnz loop
    dbr r1
    dbr r0
    hlt

loop:
    add r0, r1
    dbr r0

    mov r0, acc
    dbr r0

    mul r1, acc
    mov r1, acc
    dbr r1

    sub r0, r1
    mov r0, acc
    dbr r0

    jmp main