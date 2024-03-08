;; Example of special instruction
movi r0, 0
movi r1, 5000

main:
    vlad
    addv r0, 1
    mov r0, acc
    cmp r0, r1
    jmp main
    
hlt