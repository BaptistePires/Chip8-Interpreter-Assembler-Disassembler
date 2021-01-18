#include "chip8.hh"



chip8::chip8() {
    memset(registers, 0, COUNT_REG);
    memset(stack, 0, STACK_SIZE * sizeof(stack));
    std::fill(std::begin(keyboard), std::end(keyboard), false);
    memset(display, 0, DISPLAY_SIZE);

    mem = new uint8_t[MEM_SIZE];
    pc = MEM_START;
    sp = 0;
    disassF = false;
    // Load fonts
    for(int i = 0; i < FONT_COUNT; ++i) mem[FONT_START_ADDR + i] = chip8_fontset[i];
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

    fSize = f.tellg();
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
    /*if(debug)
        for(size_t i = 0; i < fSize; ++i) {
            std::cout << std::hex << (int) mem[MEM_START + i] << " ";
            if((i+1)%10 == 0) std::cout << std::endl;
        }*/
    return true;

} 


void chip8::run() {
    if(!init()){
        std::cout << "Error while initiating chip8, leaving..." << std::endl;
    }
    refresh();
    render();
    
    

    std::thread inputT([&]() {
        while(running) {
            int ch;
            uint8_t kId;
            switch ((ch = getch())) {
            // ESC
            case 27:
                running = false;
                continue;
                break;
            // a
            case 'a':
                kId = 0x0;
                break;
            
            case 'z':
                kId = 0x1;
                break;
            
            case 'e':
                kId = 0x2;
                break;

            case 'q':
                kId = 0x3;
                break;
            case 's':
                kId = 0x4;
                break;
            case 'd':
                kId = 0x5;
                break;
            case '<':
                kId = 0x6;
                break;
            case 'w':
                kId = 0x7;
                break;
            case 'x':
                kId = 0x8;
                break;
            case 'c':
                kId = 0x9;
                break;
            case 'r':
                kId = 0xA;
                break;
            case 'f':
                kId = 0xB;
                break;
            case 'v':
                kId = 0xC;
                break;
            case 't':
                kId = 0xD;
                break;
            case 'g':
                kId = 0xE;
                break;
            case 'b':
                kId = 0xF;
                break;
            }

            // Simulate key press, I don't think we can retrieve a key released event with ncurses :(. This is the best way I found to emulate that.
            keyboard[kId] = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            keyboard[kId] = false;
            
        }
    });

    std::ofstream log("logs.log");
    typedef std::chrono::time_point<std::chrono::high_resolution_clock> timer_t;
    
    timer_t lastInstTime = std::chrono::high_resolution_clock::now();
    timer_t now;
    log << clockSpeed << std::endl;
    double microSecPerInst = 1e6 / clockSpeed;
    while(running) {
        now = std::chrono::high_resolution_clock::now();
        if(std::chrono::duration_cast<std::chrono::microseconds>(now - lastInstTime).count() < microSecPerInst) continue;
        opcode = (mem[pc] << 8u) | mem[pc + 1]; pc+=2;
        (this->*opcodeTable[getCode(opcode)])();    
        render();
        lastInstTime = std::chrono::high_resolution_clock::now();
    }
    inputT.join();
    
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
    cbreak();
    keypad(stdscr, TRUE);
    int w, h;
    getmaxyx(stdscr, h, w);
    if(w < DISPLAY_WIDTH || h < DISPLAY_HEIGHT){
        std::cout << std::dec <<  "Terminal must be at least " << (int)DISPLAY_WIDTH << "x" << (int) DISPLAY_HEIGHT <<
        ".It's currently " << w << "x" << h  << std::endl;

        return false;
    } 
    running = true;
    return true;
}

void chip8::render() {
    
        for(int y = 0; y < DISPLAY_HEIGHT; ++y) {
            for(int x = 0; x < DISPLAY_WIDTH; ++x) {
                if(display[y * DISPLAY_WIDTH + x]) {
                    attron(COLOR_PAIR(W_BG_PAIR));
                    mvaddch(y, x,' ');
                    attroff(COLOR_PAIR(W_BG_PAIR));
                }else {
                    attron(COLOR_PAIR(B_BG_PAIR));
                    mvaddch(y, x,' ');
                    attroff(COLOR_PAIR(B_BG_PAIR));
                }
            }
        }
    refresh();
    needRender = false;
}

void chip8::disass() {
    disassFile = std::ofstream("./disass.asm",  std::ios::binary);
    disassFile << std::uppercase;
    if(!disassFile.is_open()) {
        std::cout << "Error while creating file : ./disass.asm" << std::endl;
        return;
    }
    disassF = true;

    uint8_t code;
    for(pc = PC_START; pc < (PC_START + fSize); pc+=2) {
        
        opcode = (mem[pc] << 8u) | mem[pc + 1];
        code = getCode(opcode);
        (this->*opcodeTable[code])();
        disassFile << std::endl;
    }


    disassFile.close();
}

void chip8::setClock(double clock) {
    clockSpeed = clock;
}