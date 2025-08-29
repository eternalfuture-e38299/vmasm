# VMAsm - Virtual Machine Assembler Toolkit

## Project Overview
VMAsm is a virtual machine-based assembler toolkit that includes a compiler, disassembler, and virtual machine implementation. The project is written in C++17 and uses the CMake build system.

## Features
- **Virtual Machine Core**: Supports 64 registers, snapshot functionality, and flow control
- **Compiler**: Compiles VMAsm source code into virtual machine bytecode
- **Disassembler**: Converts bytecode back into readable assembly code
- **Serialization Tools**: Supports saving virtual machine states to files or loading from files
- **System Calls**: Built-in system functions such as printing, exiting, and random number generation
- **Command-Line Tools**: Provides compilation, execution, and disassembly functionalities

## Syntax
```asm
// Single-line comment
/* Multi-line comment */
main:// Label definition
mov R0, 42// Register operation
sys 0, R0// System call
jmp #end // Unconditional jump to the specified label

end:
halt// Program termination
```

## Instruction Set
```asm
nop// Pseudo-instruction, no operation
jmp <immediate value/label>// Unconditional jump
mov <source value/register>, <destination register>// Write to register
add <immediate value/register>, <immediate value/register>, <destination register>// Addition
sub <minuend/register>, <subtrahend/register>, <destination register>// Subtraction
neg <immediate value/register>, <destination register> // Negation
snap_save // Save current registers to snapshot
snap_swap // Swap register values with snapshot
snap_clear // Clear snapshot
regs_clear // Clear registers
jz <register>, <immediate value/label> // Jump if zero
jnz <register>, <immediate value/label> // Jump if not zero
jg <register>, <immediate value/label> // Jump if greater than zero
jl <register>, <immediate value/label> // Jump if less than zero
halt // Halt
sys<call number> ...<parameters> // System call
```

## Quick Start
1. Clone the repository.
2. Build using CMake:
```bash
mkdir build && cd build
cmake .. && make
```