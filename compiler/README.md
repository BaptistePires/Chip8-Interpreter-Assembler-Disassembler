# c8compiler
This is a very basic and small compiler I made to compile ROM for chip8. 

## Rules
---  
 

### Preprocessor directives
You can define several preprocessor directives to tell the parser how to read your file. Each preprocessor directive should be on its own line and start with `#`.   

Preprocessor directives allowed :  
    - `#HEXDEF $ [or] 0x` : this allow you to specify how hex numbers are written.

### ASM directives
Here are the following directives that you can use.  
`.sprite [name] [height] [type]` : `height` represents the numbers of row the sprite will take. `name` is a string that can take any value. `type` allows you to specify the way you're declaring sprite bytes :  
- `s` means that you'll use string to represent bytes.
- `n` means that you'll use numbers to represent bytes.  

**Be careful, they are stored in a big-endian way**
### Exemple
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

# Instructions
- `bb` here is a single unsigned byte.
- `bbb` is a 12bits addr.
- `x` here is a reigster no (range : [0 - 0xF]).
- `y` here is a reigster no (range : [0 - 0xF]).
- `label` refers to a label you defined in your program.

> Any capital letter in opcodes is part of it and used to distinguish each.

Inst. Format | opcode | Description| 
| --- | --- | --- |
`CALL label` | `2eee` | Calls machine code. |  
`CLS` | `00E0` | Clear display. |
`RET` | `00EE` | Return from subroutine. |
`JP label` | `1bbb` | Jump to label.
`SE x, bb` | `3Xbb` | `registers[X] == N: pc+=2 `
`SNE x, bb` | `4Xbb` | `registers[X] != N: pc+=2 `
`SE x, y` | `5xy0` | `registers[X] == registers[Y]: pc+=2`
`LD x, y` | `6xbb` | `registers[x] = bb`
`ADD x, bb` | `7xbb` | `registers[x] += bb`
`LD x, y` | `8xy0` | `registers[x] = registers[y]`




`SNE X, Y` | `9XY0` | Skip next inst. if `registers[X] != registers[Y] `


# References
- [1] - https://en.wikipedia.org/wiki/CHIP-8