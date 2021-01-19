# Chip8Interpreter & disassembler
This is a personal project. Made it to learn about emulators & CPUs.  


## Dependencies
You'll need `ncurses` to run this program. You can install it with :  
`sudo apt-get install libncurses5-dev libncursesw5-dev`


## How to use it
First you'll need to compile it. You'll need `make` and `ncurses`.  
  
Currently, you can :  
- Run a chip8 program, they're located in `./res`. You can also provide your own if you want to. To run a program use :    
`./chip8Interpreter [path_to_file]`  
You can also specify the 'clock speed (inst. per sec.)' that you want. Sweet spot on mine is around `100`.
- You can also disassemble a file with :  
`./chip8Interpreter -d [path_to_file]`   
Result will be stored in `disass.asm`. File created can't be compiled, I just use it to understand roms.



## TODO
---
- Update README.md with SDL2 instead of ncurese.
- Sound
- Improve disass with labels ?
- Compiler
## References
---
Here are listed all of the websites/emulators I used during development.  
1. http://devernay.free.fr/hacks/chip8/C8TECH10.HTM - For everything available on it.
2. https://code.austinmorlan.com/austin/chip8-emulator - Mainly for graphics & used it to check my opcodes implementations.
3. https://github.com/mwales/chip8 - Took some ROMs from this repo.
4. http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/ - Took font array from here.
5.  https://www.zophar.net/pdroms/chip8/chip-8-games-pack.html - Most of the ROMs were taken here.