foo:
    mov r0, 12
    mov r1, 21
    mul r1, r0
    ret r2, 123456

entry main:
    dbr r2
    call foo
    dbr r2
    hlt