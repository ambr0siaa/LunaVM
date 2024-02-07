jmp main

plus:
    addi r1, r0
    mov r1, acc
    dbr r1
    ret

main:
    movi r0, 12 
    movi r1, 435
    dbr r0
    dbr r1

    pshr r0
    pshr r1
    call plus
    dbr r0

    hlt