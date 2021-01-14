#include "chip8.hh"


void chip8::initFunctionsTable() {
    opcodeTable[0] = &chip8::tableOc0;
    ocTable0[0] = &chip8::oc00E0;
    ocTable0[0xE] = &chip8::oc00EE;

    opcodeTable[1] = &chip8::oc1NNN;
    opcodeTable[2] = &chip8::oc2NNN;
    opcodeTable[3] = &chip8::oc3XKK;
    opcodeTable[4] = &chip8::oc4XKK;
    opcodeTable[5] = &chip8::oc5XY0;
    opcodeTable[6] = &chip8::oc6XKK;
    opcodeTable[7] = &chip8::oc7XKK;

    opcodeTable[8] = &chip8::tableOc8;
    ocTable8[0] = &chip8::oc8XY0;
    ocTable8[1] = &chip8::oc8XY1;
    ocTable8[2] = &chip8::oc8XY2;
    ocTable8[3] = &chip8::oc8XY3;
    ocTable8[4] = &chip8::oc8XY4;
    ocTable8[5] = &chip8::oc8XY5;
    ocTable8[6] = &chip8::oc8XY6;
    ocTable8[7] = &chip8::oc8XY7;
    ocTable8[0xE] = &chip8::oc8XY7;

    opcodeTable[9] = &chip8::oc9XY0;
    opcodeTable[0xA] = &chip8::ocANNN;
    opcodeTable[0xB] = &chip8::ocBNNN;
    opcodeTable[0xC] = &chip8::ocCXKK;
    opcodeTable[0xD] = &chip8::ocDXYN;

    opcodeTable[0xF] = &chip8::tableOcE;
    ocTableE[1] = &chip8::ocEXA1;
    ocTableE[0xE] = &chip8::ocEX9E;

    opcodeTable[0xF] = &chip8::tableOcF;
    ocTableF[0x7] = &chip8::ocFX07;
    ocTableF[0xA] = &chip8::ocFX07;
    ocTableF[0x15] = &chip8::ocFX07;
    ocTableF[0x18] = &chip8::ocFX07;
    ocTableF[0x1E] = &chip8::ocFX07;
    ocTableF[0x29] = &chip8::ocFX07;
    ocTableF[0x33] = &chip8::ocFX07;
    ocTableF[0x55] = &chip8::ocFX07;
    ocTableF[0x65] = &chip8::ocFX07;

}

void chip8::tableOc0() {
    (this->*ocTable0[getN(opcode)])();
}

void chip8::tableOc8() {
    (this->*ocTable8[getN(opcode)])();
}

void chip8::tableOcE() {
    (this->*ocTableE[getN(opcode)])();
}

void chip8::tableOcF() {
    (this->*ocTableF[getKK(opcode)])();
}


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
    uint16_t n = getN(opcode); // [0, 15]
    const uint8_t x = getX(opcode); // [0, 63]
    const uint8_t y = getY(opcode); // [0, 31]
    
    registers[0xF] = 0;
    const uint8_t xPos = x % DISPLAY_WIDTH;
    const uint8_t yPos = y % DISPLAY_HEIGHT;
    uint8_t spriteByte;
    uint8_t spritePixel, *screenPixel;
    // lines 
    for(uint8_t row = 0; row < n ; ++row) {
        
        spriteByte = mem[I + row];
        // cols
        for(uint8_t col = 0; col < 8; ++col) {
            spritePixel = spriteByte & (80 >> col);
            screenPixel = &display[(yPos + row) * DISPLAY_WIDTH + (xPos+ col)];

            if(spritePixel) {
                if(*screenPixel) registers[0xF] = 1;
                *screenPixel ^=spritePixel;
            }
        }
    }
    needRender = true;
    
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
