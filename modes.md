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
4xnn | skip if Vx != nn |
5xy0 | skip if Vx == Vy |
6xnn | Vx = nn |
7xnn | Vx += nn |
8xy0 | Vx = Vy |
8xy1 | Vx |= Vy |
8xy2 | Vx &= Vy |
8xy3 | Vx ^= Vy |
8xy4 | Vx += Vy; Vf = carry ? 1 : 0 |
8xy5 | Vx -= Vy; Vf = borrow ? 0 : 1 |
8xy6 | Vx = Vy >> 1; Vf = Vy & 0x01 |
8xy7 | Vx = Vy - Vx; Vf = borrow ? 0 : 1 |
8xye | Vx = Vy << 1; Vf = Vy & 0x80 |
9xy0 | skip if Vx != Vy |
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

## CHIP-10 (chip10)
https://chip-8.github.io/extensions/#chip-10
https://github.com/mattmikolay/viper/blob/master/volume1/issue7.pdf

Increased resolution: 128x64, otherwise identical to CHIP-8. In the original interpreter the memory from 0x055f to 0x0fff is reserved, this limitation doesn't exist in this interpreter.

## CHIP-8E (chip8e)
https://chip-8.github.io/extensions/#chip-8e
https://raw.githubusercontent.com/mattmikolay/viper/master/volume2/issue8_9.pdf

This extension was described by Gilles Detillieux in VIPER volume 2, issue 8/9. In some places the extension by Paul C. Moews from Programs for the COSMAC ELF ??? Interpreters is called CHIP-8E, however these extensions are different and not compatible.

Identical to CHIP-8, with the following opcodes added:

opcode | description | note
---|---|---
00ed | stop execution | 
00f2 | no operation | 
0151 | wait until the delay timer reaches 0 | 
0188 | skip the next instruction | 
5xy1 | skip if Vx > Vy | 
5xy2 | store Vx to Vy in memory starting at I; I = I + (x-y) + 1 |
5xy3 | load Vx to Vy from memory starting at I; I = I + (x-y) + 1 |
bbnn | jump to current instruction - nn bytes | 
bfnn | jump to current instruction + nn bytes | 
fx03 | send Vx to output port 3 |  implemented as Lua callback (output_port_3)
fx1b | skip Vx bytes | 
fx4f | delay timer = Vx; wait until the delay timer reaches 0 | 
fxe3 | wait for strobe at EF4; read Vx from input port 3 |  implemented as Lua callback (input_port_3_wait)
fxe7 | read Vx from input port 3 |  implemented as Lua callback (input_port_3)

## CHIP-8 modification for saving and restoring variables (chip8_fxf2_fx55_fx65)
https://chip-8.github.io/extensions/#chip-8-modification-for-saving-and-restoring-variables
https://github.com/mattmikolay/viper/blob/master/volume1/issue10.pdf

Extension to CHIP-8 by John Bennett, adds the fxf2 instruction to allow the following combinations:
- fxf2 + fy55: store Vx to Vy in memory starting at I; I = I + x + 1
- fxf2 + fy65: load Vx to Vy from memory starting at I; I = I + x + 1

## CHIP-8 with improved BNNN (chip8_fxf2_bnnn)
https://chip-8.github.io/extensions/#chip-8-with-improved-bnnn
https://github.com/mattmikolay/viper/blob/master/volume2/issue8_9.pdf

Extension to CHIP-8 by George Ziniewicz, adds the fxf2 instruction to allow the following combination:
- fxf2 + bnnn: jump to nnn + Vx

## CHIP-8 with FXF2 (chip8_fxf2)

A combination of chip8_fxf2_fx55_fx65 and chip8_fxf2_bnnn:
- fxf2 + fy55: store Vx to Vy in memory starting at I; I = I + x + 1
- fxf2 + fy65: load Vx to Vy from memory starting at I; I = I + x + 1
- fxf2 + bnnn: jump to nnn + Vx

## CHIP-48 (chip48)

Same opcodes as CHIP-8, but with quirks.

## CHIP-8 for ETI-660 (eti660)
https://chip-8.github.io/extensions/#chip-8-for-eti-660
https://archive.org/details/ETIA1981/ETI%201981-11%20November/page/n114/mode/2up

- Increased resolution: 64x48
- Programs start at 0x0600

Added opcodes:

opcode | description | note
---|---|---
0000 | stop execution | 
00f8 | display on | 
00fc | display off | 
00ff | no operation |
fx00 | set sound frequency | the correct mapping of Vx to a frequency is unknown

## CHIP-8 with color support for ETI-660 (eti660color)
https://chip-8.github.io/extensions/#chip-8-with-color-support-for-eti-660
https://archive.org/stream/ETIA1982/ETI%201982-04%20April#page/n87/mode/2up

Based on the ETI-660 version, with color.

Added opcodes:

opcode | description | note
---|---|---
07a2 | step background color | 
07c1 | enable color instructions |
27ab | set foreground color in area given by VE (x) and VF (y) to VD |

## CHIP-8 with high resolution and color support for ETI-660 (eti660color_highres)
https://chip-8.github.io/extensions/#chip-8-for-eti-660-with-high-resolution
https://archive.org/stream/ETIA1984/ETI%201984-02%20February#page/n87/mode/1up

Similar to the ETI-660 color extension:
- Increased resolution: 64x64
- Different color opcodes
- Programs start at 0x0700

Added opcodes:

opcode | description | note
---|---|---
049f | step background color | 
04a2 | enable color instructions |
04b2 | set foreground color in area given by V1 (x) and V2 (y) to V0 |

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

## CHIP-8 for COSMAC ELF (chip8elf)
https://chip-8.github.io/extensions/#chip-8-for-cosmac-elf
https://raw.githubusercontent.com/trapexit/chip-8_documentation/master/Programs%20for%20the%20COSMAC%20ELF%20-%20Paul%20C.%20Moews/Interpreters%20-%20Paul%20C.%20Moews.pdf

From Programs for the COSMAC ELF ??? Interpreters, adds a few instructions:

opcode | description | note
---|---|---
5xy1 | skip if Vx > Vy |
5xy2 | skip if Vx < Vy |
5xy3 | skip if Vx != Vy |
9xy1 | Vf,Vx = Vx * Vy |
9xy2 | Vx = Vx / Vy, Vf = remainder |
9xy3 | convert Vx,Vy to BCD stored at I,I+1,I+2,I+3,I+4 |
fx75 | output Vx to hex display | implemented as Lua callback
fx94 | set I to location of ASCII character in Vx |

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

Same as SUPER-CHIP 1.1, but all instructions behave as in CHIP-8

## SUPER-CHIP 1.1 with FX1E quirks (schip11_fx1e)
https://github.com/jameskozlowski/Chip8-C/blob/master/sctest.c8

SUPER-CHIP 1.1 with the following quirks to be compatible with the sctest test program:
- fx1e sets vF to 1 if I overflows, otherwise to 0
- fx1e causes I to overflow if it becomes greater than the memory size

## SUPER-CHIP Compatibility with FX1E quirks (schpc_fx1e)
SUPER-CHIP Compatibility with the same quirks as SUPER-CHIP 1.1 with FX1E quirks.

## SUPER-CHIP with scroll up (schip11scu)
https://chip-8.github.io/extensions/#super-chip-with-scroll-up
https://massung.github.io/CHIP-8/

Identical to SUPER-CHIP 1.1 with one added instruction:
- 00bn: scroll display n pixels up

## chip8run (chip8run)
http://chip8.sourceforge.net/
http://chip8.sourceforge.net/chip8-1.1.pdf
https://chip-8.github.io/extensions/#chip8run

Based on SUPER-CHIP 1.1 with different quirks and additional opcodes.

opcode | description | note
---|---|---
001n | stop execution with exit status n | n is ignored
00fa | enable the fx55_fx65_no_increment quirk |

## CHIP-8X (chip8x)
https://chip-8.github.io/extensions/#chip-8x
https://github.com/trapexit/chip-8_documentation/blob/master/Misc/VP580%2C%20VP585%2C%20VP590%2C%20VP595%20Instruction%20Manual%20Including%20CHIP-8X.pdf

This extension adds instructions to control the Color Card, Simple Sound and expansion hex keyboard.

- Programs start at 0x0300
- bnnn is unavailable (replaced with the bxyn instruction)

Added opcodes:

opcode | description | note
---|---|---
02a0 | step background color |
5xy1 | for each nibble in Vx, Vy: Vx = (Vx + Vy) % 8 |
bxy0 | set foreground color in area given by Vx and Vx+1 to Vy |
bxyn | set foreground color at Vx,Vx+1 for n rows to Vy |
exf2 | skip if key Vx is pressed on keyboard 2 == Vx | TODO
exf5 | skip if key Vx is not pressed on keyboard 2 == Vx | TODO
fxf8 | output Vx to port (set sound frequency) |
fxfb | wait for input from port and store it in Vx | not implemented

background colors: blue (default), black, green, red

foreground colors:
value | color
---|---
0 | black
1 | red
2 | blue
3 | violet
4 | green
5 | yellow
6 | aqua
7 | white (default)

## XO-CHIP (xochip)
https://github.com/JohnEarnest/Octo/blob/gh-pages/docs/XO-ChipSpecification.md
https://chip-8.github.io/extensions/#xo-chip

Based on SUPER-CHIP 1.1:
- 64 kiB of memory
- supports 4 colors

opcode | description | note
---|---|---
00dn | scroll display n pixels up |
5xy2 | save Vx to Vy (ascending or descending) in memory starting at I |
5xy3 | load Vx to Vy (ascending or descending) from memory starting at I |
f000 nnnn | I = nnnn |
fn01 | set active drawing planes to n |
f002 | store 16 bytes starting at I in the audio pattern buffer |
fx3a | pitch register = Vx |

## Octo (octo)
https://github.com/JohnEarnest/Octo
https://github.com/JohnEarnest/c-octo

XO-CHIP with different quirks and one added intruction:
- 0000: stop execution
