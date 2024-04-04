;; Count Fibonacci numbers
mov r0, 0
mov r1, 1

loop:
    add r0, r1
    mov  r0, r1
    mov  r1, acc
    dbr  r1
    jmp  loop