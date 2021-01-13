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

void chip8::oc7XKK(){
    registers[getX(opcode)] += getKK(opcode);
}


void chip8::oc8XY0(){
    registers[getX(opcode)] = registers[getY(opcode)];
}

void chip8::oc8XY1(){
    registers[getX(opcode)] |= registers[getY(opcode)];
}

void chip8::oc8XY2(){
    registers[getX(opcode)] &= registers[getY(opcode)];
}

void chip8::oc8XY3(){
    registers[getX(opcode)] ^= registers[getY(opcode)];
}

void chip8::oc8XY4(){
    uint8_t x = registers[getX(opcode)], y = registers[getY(opcode)];
    if((x + y) > (unsigned int) 0xFF) registers[0xF] = 1;
    else registers[0xF] = 0;
    registers[getX(opcode)] = ((x+y) & 0xFF);
}

void chip8::oc8XY5(){
    uint8_t x = registers[getX(opcode)], y = registers[getY(opcode)];
    if(x > y) registers[0xF] = 1;
    else registers[0xF] = 0;
    registers[getX(opcode)] -= registers[getY(opcode)];
}

void chip8::oc8XY6() {
    if(registers[getX(opcode)] & 0x1) registers[0xF] = 1;
    else registers[0xF] = 0;
    registers[getX(opcode)] /= 2;
}

void chip8::oc8XY7() {
    uint8_t x = registers[getX(opcode)], y = registers[getY(opcode)];
    if(y > x)registers[0xF] = 1;
    else registers[0xF] = 0;
    registers[getX(opcode)] -= registers[getY(opcode)];
}

void chip8::oc8XYE() {
    if(registers[getX(opcode)] >= 0b10000000) registers[0xF] = 1;
    else registers[0xF] = 0;
    registers[getX(opcode)] *= 2;
}

void chip8::oc9XY0() {
    if(registers[getX(opcode)] != registers[getY(opcode)]) pc +=2;
}

void chip8::ocANNN() {
    I = getNNN(opcode);
}

void chip8::ocBNNN() {
    pc = registers[0] + getNNN(opcode);
}

void chip8::ocCXKK() {
    // TODO : Check this opcode, seems weird to do it this way
    std::random_device engine;
    registers[getX(opcode)] = engine() & getKK(opcode);
}

void chip8::ocDXYN() {
    uint16_t n = getN(opcode);
    uint8_t x = getX(opcode);
    uint8_t y = getY(opcode);
    uint16_t start = I;
    uint8_t curByte;
    // TODO, hardest part but should be able to do it
    // for(uint16_t height = 0; height < y; ++height) {
    //     for(uint16_t width = 0; width < x; ++width) {
    //         curByte = mem[height]
    //     }
    // }
}

void chip8::ocEX9E() {
    if(keyboard[getX(opcode)]) pc+=2;
}

void chip8::ocEXA1(){
    if(!keyboard[getX(opcode)]) pc+=2;
}

void chip8::ocFX07(){
    registers[getX(opcode)] = delayTimer;
}

void chip8::ocFX0A(){
    if(!waitingKey) waitingKey = true;
    for(int i = 0; i < 0xF; i++){
        if(keyboard[i]) {
            registers[getX(opcode)] = i;
            waitingKey = false;
        }
    }
}

void chip8::ocFX15(){
    delayTimer = registers[getX(opcode)];
}

void chip8::ocFX18(){
    soundTimer = registers[getX(opcode)];
}

void chip8::ocFX1E(){
    I += registers[getX(opcode)];
}

void chip8::ocFX29(){

}

void chip8::ocFX33(){

}

void chip8::ocFX55(){
    memcpy(registers, &mem[I], sizeof(registers) * COUNT_REG);
}

void chip8::ocFX65(){
    memcpy(&mem[I], registers, sizeof(registers) * COUNT_REG);
}
