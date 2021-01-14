#include "chip8.hh"



chip8::chip8() {
    memset(registers, 0, COUNT_REG * sizeof(uint8_t));
    memset(stack, 0, STACK_SIZE * sizeof(uint16_t));
    std::fill(std::begin(keyboard), std::end(keyboard), false);
    memset(display, 0, DISPLAY_SIZE * sizeof(uint8_t));

    mem = new uint8_t[MEM_SIZE];
    pc = MEM_START;
    sp = 0;
    // Load fonts
    memcpy(mem, chip8_fontset, FONT_COUNT);
    initFunctionsTable();
}

chip8::~chip8(){
    delete[] mem;
}

bool chip8::loadFile(std::string&& filepath) {
    std::ifstream f(filepath, std::ios::binary | std::ios::ate);
    
    if(!f.is_open()) {
        std::cout << "Error while loading file " << filepath << "." << std::endl;
        return false;
    }

    size_t fSize = f.tellg();
    if(fSize > 0xFFF - 0x600) {
        std::cout << "File too big sorry " << std::endl;
        f.close();
        return false;
    }

    f.seekg(0);
    char *buf = new char[fSize];
    f.read(buf, fSize);
    if(!f.good()) {
        delete[] buf;
        std::cout << "Error while reading file's content" << std::endl;
        f.close();
        return false;
    }

    f.close();

    memcpy(&mem[MEM_START], buf, fSize);

    // Used to debug
    if(debug)
        for(size_t i = 0; i < fSize; ++i) {
            std::cout << std::hex << (int) mem[MEM_START + i] << " ";
            if((i+1)%10 == 0) std::cout << std::endl;
        }
    return true;

} 


void chip8::run() {
    opcode = mem[pc++] << 8;
    opcode |= mem[pc++];
    (this->*opcodeTable[getCode(opcode)])();
}


