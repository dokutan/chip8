# chip8 emulator emulator
A WIP implementation of CHIP-8/SUPER-CHIP/CHIP-8X/CHIP-8E/XO-CHIP/…

The goal of this program is compatibility with a large number of different CHIP-8 implementations and behaviours. This is achieved through the use of emulation [modes](modes.md), which describe the features of the emulated hardware, the available instructions and the quirks which change their behaviour.

CPU emulation (e.g. RCA 1802) is currently not part of this project. The same applies to a debugger or (dis)assembler, due to the vast amount of good existing programs for these purposes.

## Compiling
Required are SDL2 and a compiler supporting C++20, clone this repository, then run
```
make
```

## Running
```
./chip8 <mode> program.c8
./chip8 modes/schip11.lua program.c8 # This runs program.c8 in the SUPER-CHIP 1.1 mode
```

## Configuration
Colors, fonts, quirks, … can be configured by (copying and) editing the mode definitions in ``modes``.

## TODO
- make keys, scaling configurable
- add support for mapping the framebuffer to memory
- add more modes
