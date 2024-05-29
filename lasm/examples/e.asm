;; Calculate e^x

const limit   = 100.0
const default = 1.0!

;; Args:         in regsiter f0 put number of power
;; Return value: returns in register accf end value
exp:
    ;; The main idea of calculating is Taylor series for e^x
    ;; For optimize Taylor series find connection of the n-th element with the previous one:
    ;;                       a[n] = a[n-1] * x / n

    ;; x - power
    ;; n - number of series element

    mov f1, &default  ;; result
    mov f2, &default  ;; a0
    mov f3, &default  ;; n
    mov f4, &limit    ;; limit of iterations

    loop:
        ;; calculate next element
        mul f2, f0
        div f2, f3
        
        ;; add for current summ next element
        add f1, f2

        ;; compare n with limit
        ;; note: doing compare in this place
        ;;       because if limit is 1.0 all algorithm breaks
        cmp f4, f3

        ;; increment n
        add f3, &default
        jz loop

    ret accf, f1

entry main:
    mov  f0, 1.0
    call exp
    dbr  accf
    hlt