#include "chip8.hh"

void chip8::oc00E0(){
    memset(display, 0, DISPLAY_SIZE * sizeof(display));
}

void chip8::oc00EE(){
    pc = stack[sp--];
}

void chip8::oc1NNN(){
    pc = getNNN(opcode);
}   

void chip8::oc2NNN(){
    stack[sp++] = pc;
    pc = getNNN(opcode);
}

void chip8::oc3XKK(){
    if(registers[getX(opcode)] == getKK(opcode)) pc+=2;    
}

void chip8::oc4XKK(){
    if(registers[getX(opcode)] != getKK(opcode)) pc+=2;
}

void chip8::oc5XY0(){
    if(registers[getX(opcode)] == registers[getY(opcode)]) pc+=2;
}

void chip8::oc6XKK(){
    registers[getX(opcode)] = getKK(opcode);
}
