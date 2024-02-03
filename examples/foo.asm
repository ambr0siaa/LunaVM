;; genarate Fibonacci numbers
main:   
    movi r0, 0
    movi r1, 1
    jmp  loop

loop:
    addi r0, r1
    mov r1, r0
    mov r0, acc
    dbr r0
    jmp loop