#include "chip8.hh"


void chip8::initFunctionsTable() {
    instructionTable0[0x0] = {"CLS", instruction_t::type_t::NO_ARG, &chip8::oc00E0};
    instructionTable0[0xE] = {"RET", instruction::type_t::NO_ARG, &chip8::oc00EE};

    instructionTable8[0x0] = {"LD V%01.1X, V%01.1X", instruction_t::type_t::REG_REG, &chip8::oc8XY0};
    instructionTable8[0x1] = {"OR V%01.1X, V%01.1X", instruction_t::type_t::REG_REG, &chip8::oc8XY1};
    instructionTable8[0x2] = {"AND V%01.1X, V%01.1X", instruction_t::type_t::REG_REG, &chip8::oc8XY2};
    instructionTable8[0x3] = {"XOR V%01.1X, V%01.1X", instruction_t::type_t::REG_REG, &chip8::oc8XY3};
    instructionTable8[0x4] = {"ADD V%01.1X, V%01.1X", instruction_t::type_t::REG_REG, &chip8::oc8XY4};
    instructionTable8[0x5] = {"SUB V%01.1X, V%01.1X", instruction_t::type_t::REG_REG, &chip8::oc8XY5};
    instructionTable8[0x6] = {"SHR V%01.1X, V%01.1X}", instruction_t::type_t::REG_REG, &chip8::oc8XY6};
    instructionTable8[0x7] = {"SUBN V%01.1X, V%01.1X", instruction_t::type_t::REG_REG, &chip8::oc8XY7};
    instructionTable8[0xE] = {"SHL V%01.1X, V%01.1X}", instruction_t::type_t::REG_REG, &chip8::oc8XYE};

    instructionTableE[0x1] = {"SKNP V%01.1X", instruction_t::type_t::REG, &chip8::ocEXA1};
    instructionTableE[0xE] = {"SKP V%01.1X", instruction_t::type_t::REG, &chip8::ocEX9E};

    instructionTableF[0x07] = {"LD V%01.1X, DT", instruction_t::type_t::REG, &chip8::ocFX07};
    instructionTableF[0x0A] = {"LD V%01.1X, K", instruction_t::type_t::REG, &chip8::ocFX0A};
    instructionTableF[0x15] = {"LD DT, V%01.1X", instruction_t::type_t::REG, &chip8::ocFX15};
    instructionTableF[0x18] = {"LD ST, V%01.1X", instruction_t::type_t::REG, &chip8::ocFX18};
    instructionTableF[0x1E] = {"ADD I, V%01.1X", instruction_t::type_t::REG, &chip8::ocFX1E};
    instructionTableF[0x29] = {"LD F, V%01.1X", instruction_t::type_t::REG, &chip8::ocFX29};
    instructionTableF[0x33] = {"LD B, V%01.1X", instruction_t::type_t::REG, &chip8::ocFX33};
    instructionTableF[0x55] = {"LD [I], V%01.1X", instruction_t::type_t::REG, &chip8::ocFX55};
    instructionTableF[0x65] = {"LD V%01.1X, [I]", instruction_t::type_t::REG, &chip8::ocFX65};
    
}

void chip8::tableOc0() {
    uint8_t n = getN(opcode);
        if(n == 0 || n == 0xE)
            (this->*instructionTable0[getN(opcode)].func)();
}

void chip8::tableOc8() {
    uint8_t n = getN(opcode);
    if((n >= 0 && n <= 7) || n == 0xE)
        (this->*instructionTable8[getN(opcode)].func)();
}

void chip8::tableOcE() {
    uint8_t n = getN(opcode);
    if(n == 0xE || n == 0x1)
        (this->*instructionTableE[getN(opcode)].func)();
}

void chip8::tableOcF() {
    uint8_t kk = getKK(opcode);
    for(const uint8_t& fCode: opcodesF)
        if(fCode == kk)
            (this->*instructionTableF[getKK(opcode)].func)();
}


void chip8::oc00E0(){
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " CLS"<< std::endl;
        return;
    }
    memset(display, 0, sizeof(display[0]) * DISPLAY_SIZE);
}

void chip8::oc00EE(){
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " RET"<< std::endl;
        return;
    }
    pc = stack[--sp];
}

void chip8::oc1NNN(){
    if(disassF) {
        
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " JP   $" << std::hex << getNNN(opcode)<< std::endl;
        return;
    }
    pc = getNNN(opcode);
}   

void chip8::oc2NNN(){
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " CALL $" << std::hex << getNNN(opcode)<< std::endl; 
        return;
    }
    
    stack[sp++] = pc;
    pc = getNNN(opcode);
}

void chip8::oc3XKK(){
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " SE   V" << std::hex << getX(opcode) << ", $" << getKK(opcode)<< std::endl;
        return;
    }
    if(registers[getX(opcode)] == getKK(opcode)) pc+=2;    
}

void chip8::oc4XKK(){
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " SNE  V" << std::hex << getX(opcode) << ", $" << getKK(opcode)<< std::endl;
        return;
    }
    if(registers[getX(opcode)] != getKK(opcode)) pc+=2;
}

void chip8::oc5XY0(){
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " SE   V" << std::hex << getX(opcode) << ", V" << getY(opcode)<< std::endl;
        return;
    }
    if(registers[getX(opcode)] == registers[getY(opcode)]) pc+=2;
}

void chip8::oc6XKK(){
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " LD   V" << std::hex << getX(opcode) << ", $" << getKK(opcode)<< std::endl;
        return;
    }
    registers[getX(opcode)] = getKK(opcode);
}

void chip8::oc7XKK(){
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " ADD  V" << std::hex << getX(opcode) << ", $" << getKK(opcode)<< std::endl;
        return;
    }
    registers[getX(opcode)] += getKK(opcode);
}


void chip8::oc8XY0(){
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " LD   V" << std::hex << getX(opcode) << ", V" << getY(opcode)<< std::endl;
        return;
    }
    registers[getX(opcode)] = registers[getY(opcode)];
}

void chip8::oc8XY1(){
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " OR  V" << std::hex << getX(opcode) << ", V" << getY(opcode)<< std::endl;
        return;
    }
    registers[getX(opcode)] |= registers[getY(opcode)];
}

void chip8::oc8XY2(){
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " AND V" << std::hex << getX(opcode) << ", V" << getY(opcode)<< std::endl;
        return;
    }
    registers[getX(opcode)] &= registers[getY(opcode)];
}

void chip8::oc8XY3(){
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " XOR V" << std::hex << getX(opcode) << ", V" << getY(opcode)<< std::endl;
        return;
    }
    registers[getX(opcode)] ^= registers[getY(opcode)];
}

void chip8::oc8XY4(){
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " ADD V" << std::hex << getX(opcode) << ", V" << getY(opcode)<< std::endl;
        return;
    }
    uint8_t x = registers[getX(opcode)], y = registers[getY(opcode)];
    if((x + y) > 255) registers[0xF] = 1;
    else registers[0xF] = 0;
    registers[getX(opcode)] = ((x+y) & 0xFFu);
}

void chip8::oc8XY5(){
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " SUB V" << std::hex << getX(opcode) << ", V" << getY(opcode)<< std::endl;
        return;
    }
    uint8_t x = registers[getX(opcode)], y = registers[getY(opcode)];
    if(x > y) registers[0xF] = 1;
    else registers[0xF] = 0;
    registers[getX(opcode)] -= registers[getY(opcode)];
}

// according to : https://github.com/mattmikolay/chip-8/issues/4
void chip8::oc8XY6() {
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " SHR V" << std::hex << getX(opcode) << ", V" << getY(opcode)<< std::endl;
        return;
    }
    registers[0xF] = (registers[getY(opcode)] & 0x1);
    registers[getX(opcode)] = registers[getY(opcode)] >> 1;
}

void chip8::oc8XY7() {
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " SUBN V" << std::hex << getX(opcode) << ", V" << getY(opcode)<< std::endl;
        return;
    }
    uint8_t x = registers[getX(opcode)], y = registers[getY(opcode)];
    if(y > x)registers[0xF] = 1;
    else registers[0xF] = 0;
    registers[getX(opcode)] = y - x;;
}

void chip8::oc8XYE() {
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " SHL V" << std::hex << getX(opcode) << ", V" << getY(opcode)<< std::endl;
        return;
    }
    uint8_t x = getX(opcode);
    registers[0xF] = (registers[x] & 0x80u) >> 7u;
    registers[x] = registers[getY(opcode)] << 1; 

}

void chip8::oc9XY0() {
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " SNE V" << std::hex << getX(opcode) << ", V" << getY(opcode)<< std::endl;
        return;
    }
    if(registers[getX(opcode)] != registers[getY(opcode)]) pc +=2;
}

void chip8::ocANNN() {
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " LD   I" << std::hex << ", $" << getNNN(opcode)<< std::endl;; 
        return;
    }

    I = getNNN(opcode);
}

void chip8::ocBNNN() {
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " JP  V0" << std::hex << ", $" << getNNN(opcode)<< std::endl;
        return;
    }
    pc = registers[0] + getNNN(opcode);
}

void chip8::ocCXKK() {
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " RND  V" << std::hex << getX(opcode) << ", $" << getKK(opcode)<< std::endl; 
        return;
    }
    // TODO : Check this opcode, seems weird to do it this way
    std::random_device engine;
    registers[getX(opcode)] = (rand()%0xFFu) & getKK(opcode);
}

void chip8::ocDXYN() {
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " DRW  V" << std::hex << getX(opcode) << ", V" << getY(opcode) << ", $" << getN(opcode)<< std::endl; 
        return;
    }
	uint8_t Vx = getX(opcode);
	uint8_t Vy = getY(opcode);
	uint8_t height = getN(opcode);

	// Wrap if going beyond screen boundaries
	uint8_t xPos = registers[Vx] % DISPLAY_WIDTH;
	uint8_t yPos = registers[Vy] % DISPLAY_HEIGHT;

	registers[0xF] = 0;

	for (unsigned int row = 0; row < height; ++row)
	{
		uint8_t spriteByte = mem[I + row];

		for (unsigned int col = 0; col < 8; ++col)
		{
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &display[(yPos + row) * DISPLAY_WIDTH + (xPos + col)];

			// Sprite pixel is on
			if (spritePixel)
			{
				// Screen pixel also on - collision
				if (*screenPixel == 0xFFFFFFFF)
				{
					registers[0xF] = 1;
				}

				// Effectively XOR with the sprite pixel
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
    render();
}

void chip8::ocEX9E() {
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " SKP  V" << std::hex << getX(opcode)<< std::endl;
        return;
    }
    if(keyboard[registers[getX(opcode)]]) pc+=2;
}

void chip8::ocEXA1(){
    if(disassF) {
        
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " SKNP V" << std::hex << getX(opcode)<< std::endl;
        return;
    }
    // std::cout << std::hex << registers[getX(opcode)] std::endl;
    if(!keyboard[registers[getX(opcode)]]) pc+=2;
}

void chip8::ocFX07(){
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " LD   V" << std::hex << getX(opcode) << ", DT"<< std::endl;
        return;
    }
    registers[getX(opcode)] = delayTimer;
}

void chip8::ocFX0A(){
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " SKP  V" << std::hex << getX(opcode) << ", K"<< std::endl;
        return;
    }
    for(int i = 0; i < 0xF; i++){
        if(keyboard[i]) {
            registers[getX(opcode)] = i;
            return;
        }
    }
    pc-=2;
}

void chip8::ocFX15(){
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " LD   DT, V" << std::hex << getX(opcode)<< std::endl;
        return;
    }
    delayTimer = registers[getX(opcode)];
}

void chip8::ocFX18(){
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " LD   ST, V" << std::hex << getX(opcode)<< std::endl;
        return;
    }
    soundTimer = registers[getX(opcode)];
}

void chip8::ocFX1E(){
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " ADD  I, V" << std::hex << getX(opcode)<< std::endl;
        return; 
    }
}

void chip8::ocFX29(){
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " LD    F, V" << std::hex << getX(opcode)<< std::endl;
        return;
    }
    I = FONT_START_ADDR + (registers[getX(opcode)] * 5);
}

void chip8::ocFX33(){
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " LD   B, V" << std::hex << getX(opcode)<< std::endl;
        return;
    }
    uint8_t v = registers[getX(opcode)];
    mem[I]     = v/ 100;
    mem[I + 1] = (v / 10) % 10;
    mem[I + 2] = (v % 100) % 10;	
}

void chip8::ocFX55(){
    std::cout << "fx55" << std::endl;
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " LD  [I], V" << std::hex << getX(opcode)<< std::endl;
        return;
    }
    for(int i = 0; i < getX(opcode) + 1; ++i ) {
        mem[I + i] = registers[i];
    }
    
}

void chip8::ocFX65(){
    if(disassF) {
        disassFile << "[$" << std::hex << (pc & 0xFFF) << "]"  << " LD   V" << std::hex << getX(opcode) << ", [I]"<< std::endl;
        return;
    }
    for(int i = 0; i <= getX(opcode); ++i ) {
        registers[i] = mem[I + i];
    }
}

void chip8::nop(){ 
}