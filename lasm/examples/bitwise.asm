entry main:
    mov r0, 1
    mov r1, 1

    and r0, 1
    dbr r0
    and r0, r1
    dbr r0

    or r0, 1
    dbr r0
    or r0, r1
    dbr r0

    xor r0, 1
    dbr r0
    xor r0, r1
    dbr r0

    shr r0, 1
    dbr r0
    shr r0, r1
    dbr r0

    shl r1, 1
    dbr r1
    shl r1, r1
    dbr r1

    mov r2, 16
    not r2
    dbr r2

    hlt