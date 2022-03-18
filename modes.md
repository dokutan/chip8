# List of modes

This is a list of supported interpeter modes/CHIP-8 extensions. The instruction sets are listed to describe the features of this interpreter, please refer to the original sources for an accurate description. The name in parantheses is understood by the mode option.

## CHIP-8 (chip8)
https://chip-8.github.io/extensions/#chip-8
https://github.com/mattmikolay/chip-8/wiki/CHIP%E2%80%908-Instruction-Set

The original CHIP-8: 
- resolution: 64 x 32, 2 colors
- program starts at 0x200

opcode | description | note
---|---|---
0000 | unknown opcode / end of program |
00e0 | clear screen |
00ee | return |
0nnn | call machine language subroutine at nnn | not implemented
1nnn | jump to nnn |
2nnn | call subroutine at nnn |
3xnn | skip if Vx == nn |
3xnn | skip if Vx != nn |
5xy0 | skip if Vx == Vy |
6xnn | Vx = nn |
6xnn | Vx += nn |
8xy0 | Vx = Vy |
8xy1 | Vx |= Vy |
8xy2 | Vx &= Vy |
8xy3 | Vx ^= Vy |
8xy4 | Vx += Vy; Vf = carry ? 1 : 0 |
8xy5 | Vx -= Vy; Vf = borrow ? 0 : 1 |
8xy6 | Vx = Vy >> 1; Vf = Vy & 0x01 |
8xy7 | Vx = Vy - Vx; Vf = borrow ? 0 : 1 |
8xye | Vx = Vy << 1; Vf = Vy & 0x80 |
5xy0 | skip if Vx != Vy |
annn | I = nnn |
bnnn | jump to nnn + V0 |
cxnn | Vx = random & nn |
dxyn | draw n bytes at (Vx, Vy) |
ex9e | skip if pressed key == Vx |
exa1 | skip if pressed key != Vx |
fx07 | Vx = delay timer |
fx0a | wait for keypress; Vx = key |
fx15 | delay timer = Vx |
fx18 | sound timer = Vx |
fx1e | I += Vx |
fx29 | I = address of sprite of hex digit in Vx |
fx33 | memory[I, I+1, I+2] = BCD of Vx |
fx55 | store V0 to Vx in memory starting at I; I = I + x + 1 |
fx65 | load V0 to Vx from memory starting at I; I = I + x + 1 |

## CHIP-8E (chip8e)
https://chip-8.github.io/extensions/#chip-8e
https://raw.githubusercontent.com/mattmikolay/viper/master/volume2/issue8_9.pdf

Identical to CHIP-8, with the following opcodes added:

opcode | description | note
---|---|---
00ed | stop execution | 
00f2 | no operation | 
0151 | wait until the delay timer reaches 0 | 
0188 | skip the next instruction | 
5xy1 | skip if Vx > Vy | 
5xy2 | store Vx to Vy in memory starting at I; I = I + x + 1 | 
5xy3 | load Vx to Vy from memory starting at I; I = I + x + 1 | 
bbnn | jump to current instruction - nn bytes | 
bfnn | jump to current instruction + nn bytes | 
fx03 | send Vx to output port 3 | not implemented
fx1b | skip Vx bytes | 
fx4f | delay timer = Vx; wait until the delay timer reaches 0 | 
fxe3 | wait for strobe at EF4; read Vx from input port 3 | not implemented
fxe7 | read Vx from input port 3 | not implemented

## CHIP-48 (chip48)

Same opcodes as CHIP-8, but with quirks. 

## SUPER-CHIP 1.0 (schip10)
https://chip-8.github.io/extensions/#super-chip-10

- Increased resolution: 128x64
- Same quirks as CHIP-48
- opcode fx29 has support for large digit sprites
- dxy0 draws a 16x16 sprite in high resolution mode

Added opcodes:

opcode | description | note
---|---|---
00fd | stop execution | 
00fe | disable high resolution mode | 
00ff | enable high resolution mode | 
fx75 | store V0 - Vx in RPL user flags (0 <= x <= 7) |
fx85 | load V0 - Vx from RPL user flags (0 <= x <= 7) |

## SUPER-CHIP 1.1 (schip11)
http://devernay.free.fr/hacks/chip8/schip.txt
https://chip-8.github.io/extensions/#super-chip-11

- Mostly identical (added opcodes, quirks) to SUPER-CHIP 1.0
- fx29 doesn't support large digit sprites

Added opcodes:

opcode | description | note
---|---|---
00cn | scroll display n pixels down |
00fb | scroll display 4 pixels right |
00fc | scroll display n pixels left |
fx30 | I = address of large sprite of digit in Vx |

## SUPER-CHIP Compatibility (schpc)
https://chip-8.github.io/extensions/#schip-compatibility-schpc-and-gchip-compatibility-gchpc

- Same as SUPER-CHIP 1.1, but all instructions behave as in CHIP-8

