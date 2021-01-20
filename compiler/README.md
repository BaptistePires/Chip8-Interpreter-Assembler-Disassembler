# c8compiler
This is a small compiler I'm doing for me. It'll only compile files that respect the following rules.  

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