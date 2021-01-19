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
`.sprite [height] [type]` : height represents the numbers of row the sprite will take. `type` 