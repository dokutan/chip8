# chip8 emulator emulator
A WIP implementation of CHIP-8/SUPER-CHIP/CHIP-8X/CHIP-8E/XO-CHIP/…

The goal of this program is compatibility with a large number of different CHIP-8 implementations and behaviours. This is achieved through the use of emulation [modes](modes.md), which describe the features of the emulated hardware, the available instructions and the quirks which change their behaviour.

## Compiling
Required are SDL2 and a compiler supporting C++20, clone this repository, then run
```
make
```

## Running
```
./chip8 <mode> program.c8
./chip8 schip11 program.c8 # This runs program.c8 in the SUPER-CHIP 1.1 mode
```

## TODO
- implement advanced sound features (CHIP-8E, XO-CHIP)
- improve XO-CHIP compatibility
- make speed, keys, palette configurable
- add more modes
