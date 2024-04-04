# Cpu instructions usage

## Note:
All intruction use intel notation - ```Destination before source```.

## Inst Table

|Inst|What do|Option1|Option2|Option3|
|-   |-      |-      |-      |-      |
|`mov`|copy value to regester|register to register `mov r1, r0`|value to register `mov r0, 12`|copy value from previous stack frame `mov r0, $1`|
|`add`|sums 2 values and put into accumulator `acc` or `accf`|register and register `add r4, r2`| register and value `add r3, 2134`| None|
|`sub`|subtracts 2 values from left to right and put into accumulator `acc` or `accf`|register and register `sub r5, r7`| register and value `sub r2, 66`| None|
|`div`|divides 2 values from left to right and put into accumulator `acc` or `accf`|register and register `div r1, r2`| register and value `div f6, 23.7`| None|
|`mul`|multiplies 2 values from left to right and put into accumulator `acc` or `accf`|register and register `mul f1, f2`| register and value `mul r0, 4`| None|
|`jmp`|change intruction pointer to label address or direct address unconditionally|None|None|None|
|`jnz`|change intruction pointer to label address or direct address if `zero flag` is not zero|None|None|None|
|`jz`|change intruction pointer to label address or direct address if `zero flag` is zero|None|None|None|
|`ret`|load previous machine state from old stack frame and change intruction pointer to old and also can return some value to register|return nothing `ret`|return regiter to regiter `ret r8, r0`|return value to register `ret f8, 27.5`|
|`dbr`|print register value to console|None|None|None|
|`hlt`|stop program|None|None|None|
|`cmp`|compares 2 registers|None|None|None|
|`pop`|decrease stack pointer and stack size by 1|delete top value `pop`|return top value to register `pop r3`|None|
|`push`|put value on stack|put register value `push r4`|put value `push 5342`|None|
|`call`|save all state of registers to stack and branches new procedure and move to new frame|None|None|None|
|`vlad`|print in console special message|None|None|None|