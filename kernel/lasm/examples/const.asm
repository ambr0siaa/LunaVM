.constant a {i64: 12}
.constant b {i64: 45}
.constant c {f64: 123.45}

summ:
    add r0, r1
    mov r0, acc
    dbr r0
    ret

.entry main:
    mov r0, &a * 2 + (7 - 1) * (7 + 1)
    mov r1, ((&b * &a - 2) - 3 * (&b - 1) * (&b + 1)) * &a
    dbr r1

    call summ
    
    push &c * 2.0 - 1.0
    pop f0
    dbr f0

    hlt