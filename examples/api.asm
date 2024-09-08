;; THIS FILE IS NOT COMPILABLE FOR NOW!!!
;; Its just my ideas for kinds of API tha Luna should have

;; Import system for importing files. In future I'll implement something like `stdlib`.
;; Also it should have some sort of i/o, connecting with system `std::sys`,
;; strings maybe (std::str) and etc.
import std::io 

;; Note: The calling convention of label is `%name_of_label`.
;; The call starts from `%` and after this pushing name of variable
mov u32 r0, 1234
mov u32 r1, 0
label loop:
    add u32 r1, 1
    gt u32 r0, r1
    vlad
    jz %loop

;; In my idea labels is some sort of `Named memmory segment`.
;; Labels can be represents as `Named address` like `label loop:`,
;; also it can represents as some sort of `Variable`, but named
;; memmory segment.
;; 
;; The `Variable` can lacated in `static`, `dynamic` and `stack` memmory segment.
;; Default declacration of `Label-Variable` will locate on `static` memmory segment
;; I call it `data`. Example of `static` segment is below.
;;
;; The `Variables` has types (i8, u8, char, i16, u16, i32, u32, f32, i64, u64, f64).
;; All of this types can be represented as pointers. I think, to use dynamic memmory
;; segment in stdlib I'll implemented some function that allocating some memmory an heap
;; like `malloc`, `realloc`, `calloc` inc C.
;;
;; Special syntax construction is `[...]` expresions. It returns the size of type.
;; At start write type, after colon write some much size of this type you want to have.

label a u32: 234           ;; Declareted variable in static segment
label ptr0 u32 : [char:23] ;; Value of [char:23] = 1 * 23, sizeof(char) = 1
label ptr1 u32*            ;; Undeclareted variable with pointer type

;; This is special instruction that allocate on the stack (4*14) bytes
;; and pushing allocated pointer to variable `ptr1` and automatically
;; assign it to stack segment.
alloca [i32:14], %ptr1

;; Special instruciton `pull` need for working with pointers.
;; In this mode instruction means dereference of ptr0 and putting
;; there value of ptr1. In C it looks like this:
;;       int *p;
;;       int a = 234;
;;       *p = a;
pull i32 %ptr0, %ptr1
;; In this mode instruction means pushing a refrence to ptr1 in ptr0. In C it like:
;;       int *p;
;;       int a = 234;
;;       p = &a;
pull i32* %ptr0, %ptr1

;; Function declaration. It like in `llvm` but insted of `{}` I use only `end` token.
define write(i32 fd, char* buf, u32 size)
    call %builtin(1, fd, buf, size)          ;; Calling to builtin function
end

;; All function in my idea is a separate world with own static, dynamic and stack segments.
;; Declaration of label in function scope unvisible outsize of scop, it local.
define function(u32 a, b)
    label c u32 : 0
    mov u32 %c, %a
    label loop:
        mov u32 %c, %b
        mul u32 %c, 2
        sub u32 r0, %a
        mov u32 %b, r0
        ;; This is my genius fiture, I call it `Divert`. It diverting instruction
        ;; action to somewhere. In example below, result of adding of r0 and %a
        ;; moving not to r0, it moving to %c. Also it can be use for instruction.
        add u32 r0, %a -> %c
        mov u32 %b, r0
        mul u32 %b, %c
        cmp u32 r3, 123
        add u32 r3, 1
        jz %loop
    ret u32 r0
end

;; This is a module fiture. It like `namespace` in C++, but can call only functions.
module Program
    label c u32 : 12     ;; Invisible outside of scope

    define foo(i32 a, b)
        ;; Diverting for instructions. The `?` is place to where move result
        add i32 %a, %b -> mov i32 r0, ?
    end

    define buz(i32 a, b)
        add i32 %a, %b -> ret i32 ?
    end

end

label entry _start:

    label bar i32
    label hello char* : "Hello, World!\n"
    call %io::print(hello)
    call %Program::foo(r0, r1)
    dbr i32 r0 
    
    call %Program::buz(123, 53) -> %bar ;; Function result also can be divert
    
    ;; I'll implement this shit
    add i32 r0, r1 ->
        mul i32 r0, ?  ->
            sub i32 r2, ?

    dbr i32 r2
hlt
