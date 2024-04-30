;; Count Fibonacci numbers
mov r0, 0
mov r1, 1

loop:
    mov r2, r1
    add r1, r0
    mov r0, r2
    dbr r1
    jmp loop