# c8compiler
This is a very basic and small compiler I made to compile ROM for chip8. The syntax is the same as defined [here](devernay.free.fr/hacks/chip8/C8TECH10.HTM).

## Rules
---  
 
### Preprocessor directives
You can define several preprocessor directives to tell the parser how to read your file. Each preprocessor directive should be on its own line and start with `#`.   

Preprocessor directives allowed :  
    - `#HEXDEF $ [or] 0x` : this allow you to specify how hex numbers are written.

### ASM directives
Here are the following directives that you can use.  

### Sprites
`.sprite [name] [height] [type]` : `height` represents the numbers of row the sprite will take. `name` is a string that can take any value. `type` allows you to specify the way you're declaring sprite bytes :  
- `s` means that you'll use string to represent bytes.
- `n` means that you'll use numbers to represent bytes.  

**Be careful, they are stored in a big-endian way**
#### Exemple
```assembly
#HEXDEF $

.sprite s1 $4 s
"........"
"........"
"........"
"........" 

.sprite s2 $4 n
$FF
$FF
$FF
$FF
```
These will generate the same sprite.

## Labels
Labels allow you to create multiple parts in your code. 
> You always have to specify a `_start` label. 
They must follow these rules :  
- Always start with a `_`
- Be unique.  

#### Exemple
```assembly

_start:
    LD V1, $2
    CALL _end

_end:
    ADD V6, V6
    RET
```



# Instructions
- `n` 4bits usnigned value.
- `bb` here is a single unsigned byte.
- `bbb` is a 12bits addr.
- `x` here is a reigster no (range : [0 - 15]).
- `y` here is a reigster no (range : [0 - 15]).
- `I` refers to the index register.
- `K` is a key number.
- `F` is a value in range [0, 0xF] corresponding to a font sprite.
- `label` refers to a label you defined in your program.


> Any capital letter in opcodes is part of it and used to distinguish each.  
> When you specify a register, you need to put a `V` before the number, it can be either lower or upper case.   
Be careful, register numbers must be written in `base 10`.

Inst. Format | opcode | Description| 
| --- | --- | --- |
`CLS` | `00E0` | Clear display. |
`RET` | `00EE` | Return from subroutine. |
`JP label` | `1bbb` | Jump to label.
`CALL label` | `2EEE` | `pc = addr of label`
`SE Vx, bb` | `3Xbb` | `registers[X] == N: pc+=2 `
`SNE Vx, bb` | `4Xbb` | `registers[X] != N: pc+=2 `
`SE Vx, Vy` | `5xy0` | `registers[X] == registers[Y]: pc+=2`
`LD Vx, Vy` | `6xbb` | `registers[x] = bb`
`ADD Vx, bb` | `7xbb` | `registers[x] += bb`
`LD Vx, Vy` | `8xy0` | `registers[x] = registers[y]`
`OR Vx, Vy` | `8xy1` | `registers[x] |= registers[y]`
`AND Vx, Vy` | `8xy2` | `registers[x] &= registers[y]`
`XOR Vx, Vy` | `8xy3` | `registers[x] ^= registers[y]`
`ADD Vx, Vy` | `8xy4` | `registers[x] += registers[y]` and `registers[0xF] = carry ? 1 : 0`
`SUB Vx, Vy` | `8xy5` | `registers[x] -= registers[y]` and `registers[0xF] = carry ? 0 : 1`
`SHR Vx, Vy` | `8xy6` | `registers[0xF] = register[x] & 0x1 ; registers[x] >>= 1` 
`SUBN Vx, Vy` | `8xy0` | `registers[y] -= registers[x]` and `registers[0xF] = carry ? 0 : 1`
`SHL Vx, Vy` | `8xyE` | `registers[0xF] = register[x] & 0x80 ; registers[x] <<= 1` 
`SNE Vx, Vy` | `9XY0` | `registers[X] != registers[Y]: pc+=2 `
`LD I, bbb` | `Abbb` | `I = bbb`
`JP0 bbb` | `Bnnn` | `pc = registers[0] + bbb`
`RND Vx, bb` | `Cxbb` | `registers[x] = bb & randByte()`
`DRW Vx, Vy, n` | `Dxyn` | Draw sprite (located at `I`) of height `n` at `x, y`
`SKP Vx` | `Ex9E` | `keyboard[registers[x]]` is pressed : `pc+=2`
`SKNP Vx` | `ExA1` | `keyboard[registers[x]]` is not pressed : `pc+=2`
`LD Vx, DT` | `Fx07` | ` registers[x] = delay_timer`
`LD Vx, K` | `Fx0A` | Wait until a key is press then : `registers[x] = key_pressed`
`LD DT, Vx` | `Fx15` | `delay_timer = registers[x]`
`LD ST, Vx` | `Fx18` | `sound_timer = registers[x]`
`ADD I, Vx` | `Fx1E` | `I += registers[x]`
`LD F, Vx` | `Fx29` | `I = addr of F`
`LD B, Vx` | `Fx33` | Store `registers[x]` BCD representation in memory starting at address `I`.
`LD [I], x` | `Fx55` | Store register `0` to register `x` in memory starting at address `I`.
`LD x, [I]` | `Fx65` | Load register `0` to register `x` from memory starting at address `I`.


# References
- [1] - https://en.wikipedia.org/wiki/CHIP-8
- [2] - http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#3xkk