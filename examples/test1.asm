mov f64 r0, 234.0
mov f64 r1, 1.0
ge f64 r0, r1 
jnz %branch
    mov u32 r0, 235
    dbr u32 r0
    vlad
    hlt
label branch:
    mov u32 r0, 666
    dbr u32 r0
    hlt
