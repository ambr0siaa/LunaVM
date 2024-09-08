;; Calculate e^x. Number of power in register`r0`, return value in register `r1` result.
;; The main idea of calculating is Taylor series for e^x. For optimize Taylor series
;; find connection of the n-th element with the previous one: a[n] = a[n-1] * x / n,
;; where x - power and n - number of series element.
label exp:
    mov f64 r1, 1.0     ;; result
    mov f64 r2, 1.0     ;; a[0]
    mov f64 r3, 1.0     ;; n
    mov f64 r4, 20.0    ;; limit of iterations
    label loop:         ;; calculate next element
        mul f64 r2, r0
        div f64 r2, r3
        add f64 r1, r2
        add f64 r3, 1.0 ;; increment n
        ge f64 r3, r4   ;; compare n with limit
        jz %loop
    ret

label entry main:
    mov f64 r0, 1.0
    call %exp
    dbr f64 r1
    hlt
