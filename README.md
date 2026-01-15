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

## Building and Testing the E20 Assembler
1. Compile the E20 Assembler using gcc:
    ```bash
    gcc -Wall -o e20_assembler e20_assembler.c
    ```
    - The `-Wall` flag enables all compiler warnings to ensure code quality
2. To run/test the E20 Assembler use command:
    ```bash
    ./e20_assembler test.s
    ```
    - More generally, the command is given by
        ```bash
        ./e20_assembler {test_name}.s
        ```
    - The assembly file must always be an input parameter to the program when running the assembler
    - You can also choose to specify the name of the output file:
        ```bash
        ./e20_assembler test.s test.bin
        ```
    - More generally, the command for this is given by
        ```bash
        ./e20_assembler {test_name}.s {output_filename}.bin
        ```
    - The `/tests` directory contains test cases for validating the assembler:
        - `.s` files: Assembly source programs
        - `.bin` files: Expected machine code output
    - Each assembly file includes the expected binary output in comments at the end for reference
    - Note: If you do decide to specify the name of your output file, it is recommended that the name matches that of the input file to avoid any confusion

## The E20 Architecture
- The E20 has 8 general purpose registers:
    | Register | Purpose |
    | :--- | :--- |
    | $0 | Must always be 0 |
    | $1 - $7 | General purpose registers; Note that `jal` writes a return address to `$7` when invoked  |

### The E20 Instruction Set:
- **Instructions with three register arguements**:
    - add $regDst, $regSrcA, $regSrcB
        | 3 bits | 3 bits | 3 bits | 3 bits | 4 bits |
        | :--- | :--- | :--- | :--- | :--- |
        |  000   |regSrcA |regSrcB | regDst |  0000  |
        - Example: `add $1, $0, $5`
        - Adds the value of registers `$regSrcA` and `$regSrcB`, storing the sum in `$regDst`
        - Symbolically: R[regDst] <— R[regSrcA] + R[regSrcB]
    - sub $regDst, $regSrcA, $regSrcB
        | 3 bits | 3 bits | 3 bits | 3 bits | 4 bits |
        | :--- | :--- | :--- | :--- | :--- |
        |  000   |regSrcA |regSrcB | regDst |  0001  |
        - Example: `sub $1, $0, $5`
        - Subtracts the value of register $regSrcB from $regSrcA, storing the difference in $regDst
        - Symbolically: R[regDst] <- R[regSrcA] - R[regSrcB]
    - or $regDst, $regSrcA, $regSrcB
        | 3 bits | 3 bits | 3 bits | 3 bits | 4 bits |
        | :--- | :--- | :--- | :--- | :--- |
        |  000   |regSrcA |regSrcB | regDst |  0010  |
        - Example: `or $1, $0, $5`
        - Calculates the bitwise OR of the value of registers $regSrcA and $regSrcB, storing the result in $regDst
        - Symbolically: R[regDst] <- R[regSrcA] | R[regSrcB]
    - and $regDst, $regSrcA, $regSrcB
        | 3 bits | 3 bits | 3 bits | 3 bits | 4 bits |
        | :--- | :--- | :--- | :--- | :--- |
        |  000   |regSrcA |regSrcB | regDst |  0011  |
        - Example: `and $1, $2, $5`
        - Calculates the bitwise AND of the value of registers $regSrcA and $regSrcB, storing the result in $regDst
        - Symbolically: R[regDst] <- R[regSrcA] & R[regSrcB]
    - slt $regDst, $regSrcA, $regSrcB
        | 3 bits | 3 bits | 3 bits | 3 bits | 4 bits |
        | :--- | :--- | :--- | :--- | :--- |
        |  000   |regSrcA |regSrcB | regDst |  0100  |
        - Mnemonic: Set if less than
        - Example: `slt $1, $2, $5`
        - Compares the value of $regSrcA with $regSrcB, setting $regDst to 1 if $regSrcA is less than $regSrcB, and to 0 otherwise
        - The comparison performed is unsigned, meaning that the two operands are treated as unsigned 16-bit integers, not 2’s complement integers. Therefore, 0x0000 < 0xFFFF
        - Symbolically: R[regDst] <- (R[regSrcA] < R[regSrcB]) ? 1 : 0
    - jr $reg
        | 3 bits | 3 bits | 3 bits | 3 bits | 4 bits |
        | :--- | :--- | :--- | :--- | :--- |
        |  000   |   reg  |  000   |  000   |  1000  |
        - Mnemonic: Jump to register
        - Example: `jr $1`
        - Jumps unconditionally to the memory address in $reg
        - The jump destination is expressed as an absolute address. All 16 bits of the value of $reg are stored into the program counter.
        - Symbolically: pc <- R[reg]
- **Instructions with two register arguements**:
    - slti $regDst, $regSrc, imm
        | 3 bits | 3 bits | 3 bits | 7 bits |
        | :--- | :--- | :--- | :--- |
        |  111   |regSrc  | regDst |  imm   |
        - Mnemonic: Set if less than, immediate
        - Example 1: `slti $1, $2, some_label`
        - Example 2: `slti $1, $2, 30`
        - Compares the value of `$regSrc` with sign-extended imm, setting `$regDst` to 1 if `$regSrc` is less than `imm`, and to 0 otherwise.
        - The comparison performed is unsigned, meaning that the two operands are treated as unsigned 16-bit integers, not 2’s complement integers. Therefore, 0x0000 < 0xFFFF. This is true even though the argument is expressed as a 7-bit signed number.
        - Symbolically: R[regDst] <- (R[regSrc] < `imm`) ? 1 : 0
    - lw $regDst, imm($regAddr)
        | 3 bits | 3 bits | 3 bits | 7 bits |
        | :--- | :--- | :--- | :--- |
        |  100   |regAddr | regDst |  imm   |
        - Mnemonic: Load word
        - Example 1: `lw $1, some_label($2)`
        - Example 2: `lw $1, 5($2)`
        - Example 3: `lw $1, -1($2)`
        - Calculates a memory pointer by summing the signed number imm and the value `$regAddr`, and loads the value from that address, storing it in `$regDst`. The memory address is interpreted as an absolute address. The least significant 13 bits of the value of `$regAddr + imm` are used to index into memory.
        - Symbolically: R[regDst] <- Mem[R[regAddr] + `imm`]
    - sw $regSrc, imm($regAddr)
        | 3 bits | 3 bits | 3 bits | 7 bits |
        | :--- | :--- | :--- | :--- |
        |  101   |regAddr |regSrc  |  imm   |
        - Mnemonic: Store word
        - Example 1: `sw $1, some_label($2)`
        - Example 2: `sw $1, 5($2)`
        - Example 3: `sw $1, -1($2)`
        - Calculates a memory pointer by summing the signed number `imm` and the value `$regAddr`, and stores the value in `$regSrc` to that memory address. The memory address is interpreted as an absolute address. The least significant 13 bits of the value of `$regAddr + imm` are used to index into memory.
        Symbolically: Mem[R[regAddr] + imm] <- R[regSrc]
    - jeq $regA, $regB, imm
        | 3 bits | 3 bits | 3 bits | 7 bits |
        | :--- | :--- | :--- | :--- |
        |  110   |  regA  |  regB  |rel_imm |
        - `rel_imm` is defined by: `rel_imm = imm - pc - 1`
        Example 1: `jeq $1, $0, some_label`
        Example 2: `jeq $2, $3, 23` (jumps to address 23)
        - Compares the value of `$regA` with `$regB`. If the values are equal, jumps to the memory address defined by the address `imm`, which is encoded as the signed number `rel_imm`. The jump destination, `imm`, is encoded as a relative address, `rel_imm`. That is, when a jump is performed, the value `rel_imm` is sign-extended and added to the successor value of the program counter. Therefore, the actual address that will be jumped to is equal to the current program counter plus one plus the immediate value. This means that when encoding a `jeq` instruction to machine code, the field in the least significant seven bits must be the difference between the desired destination plus one plus the program counter.
        Symbolically: pc <- (R[regA] == R[regB]) ? pc+1+rel_imm : pc+1
    - addi $regDst, $regSrc, imm
        | 3 bits | 3 bits | 3 bits | 7 bits |
        | :--- | :--- | :--- | :--- |
        |  001   | regSrc | regDst |  imm   |
        - Mnemonic: Add immediate
        - Example 1: addi $1, $0, some_label
        - Example 2: addi $2, $2, -5
        - Adds the value of `$regSrc` and the signed number `imm`, storing the sum in `$regDst`
        - Symbolically: R[regDst] <- R[regSrc] + imm
- **Instructions with no register arguements**:
    - j imm
        | 3 bits | 13 bits |
        | :--- | :--- |
        |  010   |   imm   |
        - Mnemonic: Jump
        - Example 1: j some_label
        - Example 2: j 42
        - Jumps unconditionally to the memory address imm. The jump destination is expressed as a non-negative absolute address. All 13 bits of imm are stored into the program counter, while the most significant 3 bits of the program counter will be set to zero.
        - Symbolically: pc <- imm
    - jal imm
        | 3 bits | 13 bits |
        | :--- | :--- |
        |  011   |   imm   |
        - Mnemonic: Jump and link
        - Example 1: jal some_label
        - Example 2: jal 42
        - Stores the memory address of the next instruction in sequence in register $7, then jumps unconditionally to the memory address imm. The jump destination is expressed as a non-negative absolute address. All 13 bits of imm are stored into the program counter, while the most significant 3 bits of the program counter will be set to zero.
        - Symbolically: R[7] <- pc+1; pc <- imm
- **Pseudo-instructions**
    - movi $reg, imm
        - Example 1: `movi $2, 55`
        - Example 2: `movi $7, some_label`
        - Copies the value `imm` to the register `$reg`
        - The `movi $reg, imm` instruction is translated by the assembler as `addi $reg, $0, imm`
    - nop
        - Performs no operation, other than incrementing the program counter
        - The `nop` instruction is translated by the assembler as `add $0, $0, $0`
    - halt
        - Performs no operation at all. The program counter is not incremented, resulting in an infinite loop. By convention, this represents the end of the program.
        - The halt instruction is translated by the assembler as an unconditional jump instruction (`j`) to the current memory location
- **Assembler Directives**:
    - .fill imm
        - Example: `.fill some_label`
        - Example: `.fill 42`
        - Inserts a 16-bit immediate value directly into memory at the current location. This directive instructs the assembler to put a number into the place where an instruction would normally be. The immediate value may be specified numerically (positive, negative, or zero) or with a label.

- Note: Any bit pattern not covered by this section is not valid machine code and its interpretation is undefined

### Assembly Language Syntax
1. Comments:
    - Use a single `#` for single line comments
2. Labels:
    - Labels mark positions in code or data
    - They must:
        - Start with a letter or underscore
        - Contain only alphanumeric characters and underscores
        - End with a colon (:) when defined
    - Example:
        ```bash
        main:   # Label definition
        loop:
            addi $1, $1, 1
            j loop  # Label reference
        ```
3. White Space and Formatting:
    - Instructions can be indented with spaces or tabs
    - Multiple spaces/tabs are treated as single whitespace
    - Commas between operands are optional but recommended for clarity
    - Empty lines are ignored

### Common Errors
1. A limitation of subroutines on E20 is that the value of `$7` must be preserved for the duration of the subroutine. If the subroutine modifies `$7`, then the `jr` instruction may return to an unintended location. A corollary of the above limitation is that a subroutine cannot call another subroutine. Since the `jal` instruction modifies `$7`, this makes recursion awkward on E20. However, both of these limitation have workarounds. For example, a subroutine could save the value of `$7` into another register or into memory. It would then be free to manipulate `$7`, including by invoking other subroutines, as long as it restores the original value of `$7` before it returns.
2. It is an error to provide an immediate value, either numerically or by label, that exceeds the allowed number of bits for the immediate field of the given instruction. For example, `j 9000`cannot be assembled, since 9000 cannot be expressed as a 13-bit unsigned integer. Similarly, `jeq $0, $0, 9` cannot be assembled if it is located at address 0, since `90 − 0 − 1 = 89` cannot be expressed as a 7-bit 2’s complement integer.
3. Although a label may be declared before or after its use, it is an error to refer to a label that has not been declared at all in the program
4. It is an error to declare the same label more than once in a program. On the other hand, it is not an error to declare a label that is never referred to.
5. In the case of `jeq`, the instruction format provides only a 7-bit relative destination address, which is interpreted as a signed two's-complement number, sign-extended, and added to the 16-bit address of the subsequent instruction; this sum is stored into the program counter. This means that using only this instruction, it is impossible to jump to a location more than <code>2<sup>7-1</sup> − 1 = 0111111<sub>2</sub> = 63<sub>10</sub></code> cells ahead of the current program counter, or more than <code>−2<sup>7-1</sup> = 1000000<sub>2</sub> = −64<sub>10</sub></code> behind it.
