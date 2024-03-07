jmp main 

minus:
    movi r0, 1234
    subi r0, r1
    dbr acc
    ret

plus:
    movs r0, $0
    movs r1, $1

    dbr r0
    dbr r1

    addi r0, r1
    mov r0, acc
    dbr r0

    call minus
    dbr r1
    ret

main:
    movi r0, 12 
    movi r1, 435
    dbr r0
    dbr r1

    push 13
    push 36324
    call plus

    dbr r0
    dbr r1
    dbr r2

    hlt