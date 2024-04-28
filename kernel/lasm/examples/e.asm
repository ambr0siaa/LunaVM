;; Calculate e^x

;; Args:         in regsiter f0 put number of power
;; Return value: returns in register accf end value
exp:
    ;; The main idea of calculating is Taylor series for e^x
    ;; For optimize Taylor series find connection of the n-th element with the previous one:
    ;;                       a[n] = a[n-1] * x / n

    ;; x - power
    ;; n - number of series element

    mov f1, 1.0    ;; result
    mov f2, 1.0    ;; a0
    mov f3, 1.0    ;; n
    mov f4, 100.0  ;; limit of iterations

    loop:
        ;; calculate next element
        mul f2, f0
        mov f2, accf
        div f2, f3
        mov f2, accf
        
        ;; add for current summ next element
        add f1, f2
        mov f1, accf

        ;; compare n with limit
        ;; note: doing compare in this place
        ;;       because if limit is 1.0 all algorithm breaks
        cmp f4, f3

        ;; increment n
        add f3, 1.0
        mov f3, accf

        jz loop

    ret accf, f1

.entry main:
    mov  f0, 2.0   ;; argument for exp
    call exp
    dbr  accf
    hlt