# Chip8Interpreter
This is a personnal projet.  


## Dependencies
You'll need `ncurses` to run this program. You can install it with :  
`sudo apt-get install libncurses5-dev libncursesw5-dev`


## How to use it
First you'll need to compile it. You'll need `make` and `ncurses`.  
  
Currently, you can :  
- Run a chip8 program, they're located in `./res`. You can also provide your own if you want to. To run a program use :    
`./chip8Interpreter [path_to_file]`
- You can also disassemble a file with :  
`./chip8Interpreter [-d or --disass] [path_to_file]`   
Result will be stored in `disass.asm`.


## TODO
- Keyboard
- Sound
- Improve rendering, some ROM seems broken :)
- Compiler