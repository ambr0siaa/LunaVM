mov u32 r0, 0 
mov u32 r1, 1 

label loop:
    mov u32 r2, r1
    add u32 r1, r0
    mov u32 r0, r2
    dbr u32 r1
    jmp %loop

hlt
