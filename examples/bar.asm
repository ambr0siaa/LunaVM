movi r0, 1          ;; init regs
mov r1, r0

main: 
    cmp r1, r0
    jnz loop
    hlt

loop:
    addi r0, r1
    dbr r0

    mov r0, acc
    dbr r0

    muli r1, acc
    mov r1, acc
    dbr r1

    subi r0, r1
    mov r0, acc
    dbr r0

    jmp main