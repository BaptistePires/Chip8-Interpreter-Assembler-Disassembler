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
    endwin();
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
    if(!init()){
        std::cout << "Error while initiating chip8, leaving..." << std::endl;
    }
    refresh();
    render();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    while(running) {
        if(waitingKey) continue;
        opcode = mem[pc++] << 8;
        opcode |= mem[pc++];
        (this->*opcodeTable[getCode(opcode)])();    
        if(needRender)render();
    }
    
}

bool chip8::init() {
    if(initscr() == NULL) {
        std::cout << "Error : can't init ncurses :(" << std::endl;
        return false;
    }
    start_color();
    init_pair(W_BG_PAIR, COLOR_BLACK, COLOR_WHITE);
    init_pair(B_BG_PAIR, COLOR_WHITE, COLOR_BLACK);
    raw();
    noecho();
    running = true;
    return true;
}

void chip8::render() {
    move(0, 0);
    for(int y = 0; y < DISPLAY_HEIGHT; ++y) {
        for(int  x = 0; x < DISPLAY_WIDTH; ++x) {
            if(display[y * DISPLAY_HEIGHT + x]) attron(COLOR_PAIR(B_BG_PAIR));
            else attron(COLOR_PAIR(W_BG_PAIR));
            move(y , x);
            addch('0');
        }
    }

    refresh();
    needRender = false;
}

