jmp main

minus:
    mov r0, 1234
    sub r0, r1
    dbr acc
    ret

plus:
    mov r0, $0
    mov r1, $1

    dbr r0
    dbr r1

    add r0, r1
    mov r0, acc
    dbr r0

    call minus
    dbr r1
    ret

main:
    mov r0, 12
    mov r1, 435
    dbr r0
    dbr r1

    push 13
    push 36324
    call plus

    dbr r0
    dbr r1
    dbr r2

    hlt