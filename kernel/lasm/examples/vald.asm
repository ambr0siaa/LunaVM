;; Example of special instruction
mov r0, 0
mov r1, 5000

main:
    vlad
    add r0, 1
    mov r0, acc
    cmp r0, r1
    jz main
    
hlt