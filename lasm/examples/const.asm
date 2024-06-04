const a = 12
const b = 45
const c = 123.45

summ:
    add r0, r1
    dbr r0
    ret

;; TODO: This super mega calculations don't work - make `parse_expr` and `eval_expr` better
entry main:
    mov r0, &a * 2 + (7 - 1) * (7 + 1)
    mov r1, ((&b * &a - 2) - 3 * (&b - 1) * (&b + 1)) * &a
    dbr r1

    call summ
    
    mov r5, &a
    dbr r5

    push &c * 2.0 - 1.0
    push &b
    pop  r5
    dbr  r5

    hlt