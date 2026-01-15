# E20-Assembler
This is a two-pass assembler written in C for the E20 ISA, performing symbol resolution, pseudo-instruction expansion, and instruction encoding to generate executable 16-bit machine code suitable for CPU simulation.

## Overview
This project implements a two-pass assembler for the E20 Architecture, a teaching ISA commonly used in university Computer Architecture courses. The assembler executes two passes over the source code; the first pass builds a symbol table by recording all the labels and their addresses, and the second pass generates the actual machine code by encoding instructions and resolving the recorded symbol references.

The E20 is a 16-bit RISC architecture with 8 general-purpose registers and a simplified instruction set. This assembler is responsible for translating human readable assembly language into machine code that can be executed on an E20 simulator, which can be found here:
    > https://github.com/rikusc04/E20-CPU-Cache-Simulator

## Features
**Two Pass Assembly Algorithm**: Efficient symbol resolution for forward references
**Complete E20 Instruction Set Support**: Full support for the E20 instruction set
**Pseudo-Instruction Expansion**: Expands `movi`, `nop`, and `halt` into equivalent machine instructions
**Symbol Table Management**: Label definition and reference with case-sensitive matching
**Comprehensive Error Checking**: Validates syntax, register ranges, immediate values, and operand counts
**`.fill` directive**: Initialize memory locations with values or label addresses
**Output Format**: Verilog-compatible output format for hardware simulation and E20 CPU simulation
**Automatic Output Naming**: Auto-generates output filename from input if not specified

## Building and Running the E20 Assembler
1. Compile the E20 Assembler using gcc:
    ```bash
    gcc -Wall -o e20_assembler e20_assembler.c
    ```
    - The `-Wall` flag enables all compiler warnings to ensure code quality
2. To run the E20 Assembler use command:
    ```bash
    ./vea test.s
    ```
    - More generally, the command is given by
    ```bash
    ./vea {test_name}.s
    ```
    - The assembly file must always be an input parameter to the program when running the assembler
    - You can also choose to specify the name of the output file:
    ```bash
    ./vea test.s test.bin
    ```
    - More generally, the command for this is given by
    ```bash
    ./vea {test_name}.s {output_filename}.bin
    ```
    - Note: If you do decide to specify the name of your output file, it is recommended that the name matches that of the input file to avoid any confusion




## Testing the E20 Assembler
- Navigate to the `/tests` directory to find the tests for validating the behavior of E20 Assembler. In it, you will find the original `.s` files, as well as the `.bin` associated with each `.s` file.