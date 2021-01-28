#include "chip8.hh"


chip8::chip8() : instructionTable{
                    {"NOP", instruction_t::type_t::JP_TABLE, &chip8::tableOc0},
                    {"JP $%03.3X", instruction_t::type_t::ADDR, &chip8::oc1NNN},
                    {"CALL $%03.3X", instruction_t::type_t::ADDR, &chip8::oc2NNN},
                    {"SE V%01.1X, $%02.2X", instruction_t::type_t::REG_BYTE, &chip8::oc3XKK},
                    {"SNE V%01.1X, $%02.2X", instruction_t::type_t::REG_BYTE, &chip8::oc4XKK},
                    {"SE V%01.1X, V%01.1X", instruction_t::type_t::REG_REG, &chip8::oc5XY0},
                    {"LD V%01.1X,  $%02.2X", instruction_t::type_t::REG_BYTE, &chip8::oc6XKK},
                    {"ADD V%01.1X,  $%02.2X", instruction_t::type_t::REG_REG, &chip8::oc7XKK},
                    {"NOP", instruction_t::type_t::JP_TABLE, &chip8::tableOc8},
                    {"SNE V%01.1X, V%01.1X", instruction_t::type_t::REG_REG, &chip8::oc9XY0},
                    {"LD I, $%03.3X", instruction_t::type_t::ADDR, &chip8::ocANNN},
                    {"JP V0, $%03.3X", instruction_t::type_t::ADDR, &chip8::ocBNNN},
                    {"RND V%01.1X, $%02.2X", instruction_t::type_t::REG_BYTE, &chip8::ocCXKK},
                    {"DRW V%01.1X, V%01X, $%01.1X", instruction_t::type_t::REG_REG_NIBBLE, &chip8::ocDXYN},
                    {"NOP", instruction_t::type_t::JP_TABLE, &chip8::tableOcE},
                    {"NOP", instruction_t::type_t::JP_TABLE, &chip8::tableOcF}},
                    instructionTable0(0x10, {"NOP", instruction::type_t::NOP, &chip8::nop}),
                    instructionTable8(0x10, {"NOP", instruction::type_t::NOP, &chip8::nop}),
                    instructionTableE(0x10, {"NOP", instruction::type_t::NOP, &chip8::nop}),
                    instructionTableF(0x100, {"NOP", instruction::type_t::NOP, &chip8::nop})
{

    std::fill(std::begin(keyboard), std::end(keyboard), false);

    mem = new uint8_t[MEM_SIZE];
    pc = PC_START;
    sp = 0;
    disassF = false;
    I = 0;
    delayTimer = 255;
    soundTimer = 0;

    display = new uint32_t[DISPLAY_SIZE];
    // memset(display, 0, sizeof(display[0]) * DISPLAY_SIZE);
    clockSpeed = INST_PER_SEC;
    
    for(int i = 0; i < FONT_COUNT; ++i){
        mem[fontStartAddr + i] = chip8_fontset[i];
    }
    halt = false;
    
    initFunctionsTable();
    monitor = nullptr;
}

chip8::~chip8(){

    if(monitor != nullptr) delete monitor;
    SDL_DestroyTexture(rendererWrapper.texture);
    SDL_DestroyRenderer(rendererWrapper.r);
    SDL_DestroyWindow(rendererWrapper.w);
    SDL_Quit();
    endwin();
    delete[] mem;
    delete[] display;

}

void chip8::run() {
    if(!init()){
        std::cout << "Error while initiating chip8, leaving..." << std::endl;
    }

    typedef std::chrono::time_point<std::chrono::high_resolution_clock> timer_t;
    
    double microSecPerInst = 1e6 / clockSpeed;
    
    timer_t lastInstTime = std::chrono::high_resolution_clock::now();
    timer_t timerInsts = std::chrono::high_resolution_clock::now();
    int instCount = 0;
    timer_t now;

    std::thread t(&chipMonitor::render, monitor);
    
    while(running) {

        handleEvents();
        
        now = std::chrono::high_resolution_clock::now();
        if(std::chrono::duration_cast<std::chrono::microseconds>(now - lastInstTime).count() < microSecPerInst)continue;
        if(std::chrono::duration_cast<std::chrono::seconds>(now - timerInsts).count() >= 1) {
            timerInsts = now;
            instCount = 0;
        }
        opcode = (mem[pc] << 8u) | mem[pc + 1]; pc+=2;

        (this->*instructionTable[getCode(opcode)].func)(); 
        instCount++;

        if(delayTimer>0) delayTimer--;

        // Not really working, need to figure why
        if(soundTimer>0){
            soundTimer--;
            SDL_PauseAudio(SDL_FALSE);
        }else{
            SDL_PauseAudio(SDL_TRUE);
        }
        // std::cout << "\r R3 : " << (int) (registers[3]) << " Inst. per sec :" << std::dec << std::flush << instCount <<"." << std::flush;
        lastInstTime = std::chrono::high_resolution_clock::now();

        
    }

    t.join();

}

bool chip8::init() {

    

    if(initNcurses()) {
        monitor = new chipMonitor(this);
    }else{
        std::cout << "Can't run monitor. Please check that you have the requiered ncurses version installed.(see README.md)" << std::endl; 
    }
    running = initSDL2();

    return running;
}

bool chip8::initSDL2() {
    // Init video
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cout << "Error while SDL_Init() :" << SDL_GetError() << std::endl;
        return false;
    }


    if (SDL_CreateWindowAndRenderer(DISPLAY_WIDTH * 10, DISPLAY_HEIGHT * 10  , SDL_WINDOW_RESIZABLE |SDL_WINDOW_SHOWN, &rendererWrapper.w, &(rendererWrapper.r)) < 0) {
        std::cout << "Error while creating window and renderer : " << SDL_GetError() << std::endl;
        return false;
    }
    std::string title = "Chip-8 Interpreter - " + romPath;
    SDL_SetWindowTitle(rendererWrapper.w, title.c_str());

    rendererWrapper.texture = SDL_CreateTexture(rendererWrapper.r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, DISPLAY_WIDTH , DISPLAY_HEIGHT );
    if(rendererWrapper.texture < 0) {
        std::cout << "Error while creating texture" << std::endl;
        return false;
    }

    // Init sound & generate
    memset(&audio, 0, sizeof(audio));
    audio.spec.freq = 96000;
    audio.spec.format = AUDIO_S32SYS;
    audio.spec.channels = 1;
    audio.spec.samples = 4096;
    audio.spec.callback =  [](void* param, Uint8* stream, int len)
    {
            
        int samples = len / sizeof(float); // 4096

        for (auto i = 0; i < samples; i++){
            reinterpret_cast<float*>(stream)[i] = 0.5 * SDL_sinf(2 * M_PI * i / 1000);
        }
    };
    audio.spec.userdata = &audio.sampleNo;

    audio.device = SDL_OpenAudioDevice(nullptr, 0, &audio.spec, &audio.spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
    if(audio.device < 0) {
        std::cout << "Error while opening audio device : " << SDL_GetError();
        return false;
    }
    SDL_PauseAudio(SDL_FALSE);
    return true;
}

bool chip8::initNcurses() {
    
    initscr();
    raw();
    noecho();
    wtimeout(stdscr, -1);
    cbreak();
    nodelay(stdscr, TRUE);
    curs_set(0);
    if(has_colors() == FALSE) {
        endwin();
        std::cout << "Your terminal doesn't support colors." << std::endl;
        return false;
    }
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    return true;
}

void chip8::render() {
    SDL_RenderClear(rendererWrapper.r);
    SDL_UpdateTexture(rendererWrapper.texture, nullptr, &display[0], sizeof(display[0]) * DISPLAY_WIDTH);
    SDL_RenderCopy(rendererWrapper.r, rendererWrapper.texture, nullptr, nullptr);
    SDL_RenderPresent(rendererWrapper.r);
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
    std::cout << std::endl;
    for(pc = PC_START; pc < MEM_SIZE; pc+=2) {
        
        opcode = (mem[pc] << 8u) | mem[pc + 1];
        if(opcode == 0x0000) continue;
        
        code = getCode(opcode);
        std::cout << "opcode : " << std::hex << (int)opcode << ", code : " << (int) code << std::endl;
        if(code >=0 && code <= 0xF)
            (this->*instructionTable[code].func)();
        
    }


    disassFile.close();
}

bool chip8::loadFile(std::string&& filepath) {
    std::ifstream f(filepath, std::ios::binary | std::ios::ate);
    
    if(!f.is_open()) {
        std::cout << "Error while loading file " << filepath << "." << std::endl;
        return false;
    }

    fSize = f.tellg();
    if(fSize > 0xFFF - 0x200) {
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
    // if(debug)
    //     for(size_t i = 0; i < fSize; ++i) {
    //         std::cout << std::hex << (int) mem[MEM_START + i] << " ";
    //         if((i+1)%10 == 0) std::cout << std::endl;
    //     }

    romPath = std::move(filepath);
    return true;

} 



void chip8::setClock(double clock) {
    clockSpeed = clock;
}

void chip8::setFontAddr(int fontAddr) {
    fontStartAddr = fontAddr;
    for(int i = 0; i < FONT_COUNT; ++i){
        mem[fontStartAddr + i] = chip8_fontset[i];
    }
}

/**
 * Calling this function assumes that ncurses was correctly initiated.
 * Need to clean this code i can't even push it or i won't ever get a job lol:)
 * This is temporary obvs don't judge me pls
 **/
// void chip8::ncursesDeamon(chip8& chip) {
//     WINDOW *regWin, *ramWin;
//     int savedSize[2] = {chip.termSize[0],chip.termSize[1]};
//     int regWinWidth = (float)savedSize[0] * 0.20;
//     regWin = newwin(savedSize[1], regWinWidth, 0, 0);
//     ramWin = newwin(savedSize[1], (float)savedSize[0] * 0.80, 0, regWinWidth + 1);
//     uint16_t line;
//     uint16_t tmpPc, buffPc;
//     std::stringstream ss;
//     uint16_t currOpcode;
//     uint8_t currCode;
//     instruction_t inst;
//     char currMemo[100];
//     char currPcTxt[6];
//     bool indirection = false;
//     int offset;
//     bool halt(false);

//     int ch;
//     while(chip.running) { 


//         do {
//             // Press Enter to halt 
//             if(wgetch(stdscr) == 10) halt = !halt;
//             std::this_thread::yield();
//         }while(halt && chip.running);
               

//         // Handle resize
//         if(savedSize[0] != chip.termSize[0] || savedSize[1] != chip.termSize[1]) {
//             savedSize[0] = chip.termSize[0];
//             savedSize[1] = chip.termSize[1];
//             regWinWidth = (float)savedSize[0] * 0.20;
//             regWin = newwin(savedSize[1], regWinWidth, 0, 0);
//             ramWin = newwin(savedSize[1], (float)savedSize[0] * 0.80, 0, regWinWidth + 1);
//         }
//         // Handle registers
        
//         box(regWin, 0, 0);
//         mvwprintw(regWin, 0, 0, "Registers");
//         for(line = 1; line < 0xF && line < savedSize[1] - 1; ++line) {
            
//             mvwprintw(regWin, line, 1, "v%1X  : 0x%2X", line ,  chip.registers[line]);
//         }
//         mvwprintw(regWin, line, 1, "dt  : 0x%2X", chip.delayTimer);line++;
//         mvwprintw(regWin, line, 1, "st  : 0x%2X", chip.soundTimer);line++;
//         line++;
//         mvwprintw(regWin, line, 1, "PC  : 0x%2X", chip.pc);
//         line++;
//         mvwprintw(regWin, line, 1, "SP  : 0x%d", chip.sp);

        
//         // Handle RAM
//         box(ramWin, 0, 0);
//         mvwprintw(ramWin, 0, 0, "RAM");
//         buffPc = chip.pc;
//         offset = savedSize[1] / 2;
//         if(buffPc%2==0) offset = offset%2==0 ? offset : offset - 1;
//         else  offset = offset%2==0 ? offset - 1: offset ;

//         for(line = 1, tmpPc = buffPc - offset ; line < savedSize[1] - 1; line++, tmpPc+=2) {
            
            
//             currOpcode =(chip.mem[tmpPc] << 8) | (chip.mem[tmpPc + 1]);
//             currCode = getCode(currOpcode);
//             inst = chip.instructionTable[currCode];
//             do {
//                 indirection = false;
//                 switch(inst.type) {
//                     case instruction_t::NOP:
//                         sprintf(currMemo, inst.memonic.c_str());
//                         break;
                    
//                     case instruction_t::REG:
//                         sprintf(currMemo, inst.memonic.c_str(), getX(chip.opcode));
//                         break;
                    
//                     case instruction_t::REG_REG:
//                         sprintf(currMemo, inst.memonic.c_str(), getX(chip.opcode), getY(chip.opcode));
//                         break;

//                     case instruction_t::REG_BYTE:
//                         sprintf(currMemo, inst.memonic.c_str(), getX(chip.opcode), getKK(chip.opcode));
//                         break;
                    
//                     case instruction_t::REG_REG_NIBBLE:
//                         sprintf(currMemo, inst.memonic.c_str(), getX(chip.opcode), getY(chip.opcode), getN(chip.opcode));
//                         break;

//                     case instruction_t::ADDR:
//                         sprintf(currMemo, inst.memonic.c_str(), getNNN(chip.opcode));
//                         break;

//                     case instruction_t::NO_ARG:
//                         sprintf(currMemo, inst.memonic.c_str());
//                         break;
                    
//                     // Handle indirection
//                     case instruction_t::JP_TABLE:
//                         indirection = true;
//                         // std::cout << currCode << std::endl;
//                         switch(currCode) {
//                             case 0x0:
//                                 inst = chip.instructionTable0[getN(chip.opcode)];
//                                 break;

//                             case 0x8:
//                                 inst = chip.instructionTable8[getN(chip.opcode)];
//                                 break;
                            
//                             case 0xE:
//                                 inst = chip.instructionTableE[getN(chip.opcode)];
//                                 break;

//                             case 0xF:
//                                 inst = chip.instructionTableF[getKK(chip.opcode)];
//                                 break;

//                             default:
//                                 indirection = false;
//                         }
//                 }
//             } while(indirection);
            
            

//             if(tmpPc == buffPc) {
//                 // std::this_thread::sleep_for(std::chrono::seconds(1));
//                 wattron(ramWin, COLOR_PAIR(1));
//                 mvwprintw(ramWin, line, 1, "[$%03.3X] - ", tmpPc);
//                 mvwprintw(ramWin, line, 10, currMemo, tmpPc, currOpcode);
//                 wattroff(ramWin, COLOR_PAIR(1));
//             }else {
//                 mvwprintw(ramWin, line, 10, currMemo, tmpPc, currOpcode);
//             }
            
//         }
//         wrefresh(regWin);
//         wrefresh(ramWin);
//         std::this_thread::sleep_for(std::chrono::milliseconds(50));
//     }

//     delwin(regWin);
//     delwin(ramWin);    
// }


uint8_t* chip8::getRegisters() {
    return registers;
}

std::atomic<bool>* chip8::getRunning() {
    return &running;
}

void chip8::setRunning(bool state) {
    running = state;
}

std::atomic<bool>*  chip8::getHalt() {
    return &halt;
}

void chip8::setHalt(bool state) {
    halt = state;
}

void chip8::handleEvents() {
    SDL_Event e;
    SDL_PollEvent(&e);
    switch (e.type){

    case SDL_QUIT:
        running = false;
        break;


    case SDL_KEYDOWN:
        switch (e.key.keysym.sym) {
            case  SDLK_ESCAPE:
                running = false;
                break;
            case SDLK_0:
                keyboard[0x0] = true;
                break;
            case SDLK_1:
                keyboard[0x1] = true;
                break;
            
            case SDLK_2:
                keyboard[0x2] = true;
                break;
            case SDLK_3:
                keyboard[0x3] = true;
                break;
            case SDLK_4:
                keyboard[0x4] = true;
                break;
            case SDLK_5:
                keyboard[0x5] = true;
                break;
            case SDLK_6:
                keyboard[0x6] = true;
                break;
            case SDLK_7:
                keyboard[0x7] = true;
                break;
            case SDLK_8:
                keyboard[0x8] = true;
                break;
            case SDLK_9:
                keyboard[0x1] = true;
                break;
            case SDLK_a:
                keyboard[0xA] = true;
                break;
            case SDLK_b:
                keyboard[0xB] = true;
                break;
            case SDLK_c:
                keyboard[0xD] = true;
                break;
            case SDLK_d:
                keyboard[0xD] = true;
                break;
            case SDLK_e:
                keyboard[0xE] = true;
                break;
            case SDLK_f:
                keyboard[0xF] = true;
                break;
        }
        break;
    case SDL_KEYUP:
        switch (e.key.keysym.sym) {
            case SDLK_0:

                keyboard[0x0] = false;
                break;
            case SDLK_1:
                keyboard[0x1] = false;
                break;
            
            case SDLK_2:
                keyboard[0x2] = false;
                break;
            case SDLK_3:
                keyboard[0x3] = false;
                break;
            case SDLK_4:
                keyboard[0x4] = false;
                break;
            case SDLK_5:
                keyboard[0x5] = false;
                break;
            case SDLK_6:
                keyboard[0x6] = false;
                break;
            case SDLK_7:
                keyboard[0x7] = false;
                break;
            case SDLK_8:
                keyboard[0x8] = false;
                break;
            case SDLK_9:
                keyboard[0x1] = false;
                break;
            case SDLK_a:
                keyboard[0xA] = false;
                break;
            case SDLK_b:
                keyboard[0xB] = false;
                break;
            case SDLK_c:
                keyboard[0xD] = false;
                break;
            case SDLK_d:
                keyboard[0xD] = false;
                break;
            case SDLK_e:
                keyboard[0xE] = false;
                break;
            case SDLK_f:
                keyboard[0xF] = false;
                break;
        }
    }
            
}