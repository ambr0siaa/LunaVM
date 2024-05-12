# Luna Virtual Machine

Simple register-based virtual machine with own assembler.

* [lasm](lasm/src/lasm.c) - Assembler for the Luna.
* [lunem](cpu/src/lunem.c) - Luna's bytecode emulator.
* [dilasm](dilasm/dilasm.c) - disassembler for Luna's bytecode.

## Quick Start

To build all project at start use ```bilSetup.sh``` then run ```bil```:
```
$ ./bilSetup.sh
$ ./bin/bil -b
```

To test some example:
```
$ ./bin/bil -hdlr lasm -i <example.asm> -o <example.ln>
$ ./bin/bil -hldr lunem -i <example.ln>
```