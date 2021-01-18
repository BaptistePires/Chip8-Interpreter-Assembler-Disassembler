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
    screen = new std::bitset<4096>();
    initFunctionsTable();


}

chip8::~chip8(){
    delete[] mem;
    delete screen;
    
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
    
    

    std::thread inputT([&]() {
        while(running) {
            int ch;
            switch ((ch = getch()))
            {
            // ESC
            case 27:
                running = false;
                break;
            // a
            case 97:

                keyboard[0] = true;
                break;
            default:
                
                refresh();
                break;
            }
            
        }
    });
    std::ofstream log("logs.log", std::ios::binary);
    while(running) {
        
        opcode = (mem[pc] << 8u) | mem[pc + 1]; pc+=2;
        log << std::hex << opcode << std::endl;
        // mvprintw(0,0, "0x%04X", opcode);
        
        // std::cout << std::hex<< (int) opcode << std::endl;
        (this->*opcodeTable[getCode(opcode)])();    
        render();
        
        // refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
    
        // attron(COLOR_PAIR(B_BG_PAIR));
        // bkgdset(' ');
        // attroff(COLOR_PAIR(B_BG_PAIR));
        // for(int y = 0; y < DISPLAY_HEIGHT; ++y) {
        //     for(int  x = 0; x < DISPLAY_WIDTH; ++x) {
            
        //     if(display[y * DISPLAY_WIDTH + x] != 0){
        //         attron(COLOR_PAIR(W_BG_PAIR));
        //         mvaddch(y, x,' ');
        //         attroff(COLOR_PAIR(W_BG_PAIR));

        //     }else{continue;
        //         attron(COLOR_PAIR(B_BG_PAIR));
        //         mvaddch(y, x,' ');
        //         attroff(COLOR_PAIR(B_BG_PAIR));

        //     }
        // }      
        // }   
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
    // +2 because opcodes are 16bits.

    for(pc; pc < (PC_START + fSize); pc+=2) {
        opcode = (mem[pc] << 8u) | mem[pc + 1];
        (this->*opcodeTable[getCode(opcode)])();
        disassFile << std::endl;
    }


    disassFile.close();
}


// void chip8::disassFile(const std::string& path) {
//     std::ifstream f(path, std::ios::binary | std::ios::ate);
    
//     if(!f.is_open()) {
//         std::cout << "Error while loading file " << path << "." << std::endl;
//         return;
//     }

//     size_t fSize = f.tellg();
//     if(fSize > 0xFFF - 0x600) {
//         std::cout << "File too big sorry " << std::endl;
//         f.close();
//         return;
//     }

//     f.seekg(0);
//     char *buf = new char[fSize];
//     f.read(buf, fSize);
//     if(!f.good()) {
//         delete[] buf;
//         std::cout << "Error while reading file's content" << std::endl;
//         f.close();
//         return;
//     }
//     f.close();

//     std::ofstream disass("./disass.asm",  std::ios::binary);
//     if(!disass.is_open()) {
//         std::cout << "Error while loading file :" << path << std::endl;
//         return;
//     }
//     uint16_t opcode;
//     std::stringstream ss;
//     for(size_t i = 0; i < fSize; i+=2) {
//         opcode = buf[i] << 8;
//         opcode |= buf[i+1];
//         ss.str("");
//         ss.clear();
//         ss << "$" << std::hex << (0xFFF & i) << " " << std::dec;
//         switch(getCode(opcode)) {
//             case 0x0:
//                 if(getY(opcode) == 0xE) {
//                     ss << "CLS";
//                 }else{
//                     ss << "SYS " << std::hex << getNNN(opcode);
//                 }
//                 break;
//             default:
//                 ss << "Unknow opcode : " << std::hex << "$" << opcode;

//         }
        
//         disass << ss.str() << std::endl;
//     }

//     disass.close();
// }

