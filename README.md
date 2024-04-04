# Luna Virtual Machine

Simple register-based virtual machine with own assembler.

* [lasm](./kernel/lasm/src/lasm.c) - Assembler for the Luna.
* [lunem](./kernel/cpu/src/lunem.c) - Luna's bytecode emulator.
* [dis](./disasm/disasm.c) - disassembler for Luna's bytecode.

## Quick Start

To build all project at start use ```bilSetup.sh``` then run ```bil```:
```
$ ./bilSetup.sh
$ ./bin/bil 
```

To test some example:
```
$ ./bin/bil -hdlr lasm -i <example.asm> -o <example.ln>
$ ./bin/bil -hldr lunem -i <example.ln>
```