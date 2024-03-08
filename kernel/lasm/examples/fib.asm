;; Count Fibonacci numbers
movi r0, 0
movi r1, 1

loop:
    addi r0, r1
    mov  r0, r1
    mov  r1, acc
    dbr  r1
    jmp  loop