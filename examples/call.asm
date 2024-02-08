jmp main

minus:
    movi r0, 1234
    subi r0, r1
    dbr acc
    ret

plus:
    addi r1, r0
    mov r1, acc
    call minus
    dbr r1
    ret

main:
    movi r0, 12 
    movi r1, 435
    dbr r0
    dbr r1

    call plus

    dbr r0
    dbr r1
    dbr r2
    hlt