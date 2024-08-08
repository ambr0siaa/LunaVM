# Luna Virtual Machine

Simple register-based virtual machine with own assembler.

* [lasm](lasm/src/lasm.c) - Assembler for the Luna.
* [lunem](cpu/src/lunem.c) - Luna's bytecode emulator.
* [dilasm](dilasm/dilasm.c) - disassembler for Luna's bytecode.

## Quick Start

To build all project at start use `bilSetup.sh` then run `build`:
```
$ ./bilSetup.sh
$ ./bin/build -b
```

To test some example:
```
$ ./bin/build -hdlr lasm -i <example.asm> -o <example.ln>
$ ./bin/build -hldr lunem -i <example.ln>
```
