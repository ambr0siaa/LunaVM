.constant a {i64: 12}
.constant b {i64: 45}

summ:
    add r0, r1
    mov r0, acc
    dbr r0
    ret

.entry main:
    mov r0, &a * 2 + (7 - 1) * (7 + 1)
    mov r1, &b

    call summ

    hlt