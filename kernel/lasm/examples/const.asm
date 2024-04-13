.constant a {i64; 12}
.constant b {i64; 45}

summ:
    add r0, r1
    mov r0, acc
    dbr r0
    ret

.entry main:
    mov r0, &a
    mov r1, &b

    call summ

    hlt