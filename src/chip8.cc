#include "chip8.hh"


chip8::chip8() {

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
    
    initFunctionsTable();
}

chip8::~chip8(){

        
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

    std::thread t(ncursesDeamon, std::ref(*this));
    
    while(running) {

        handleEvents();
        
        now = std::chrono::high_resolution_clock::now();
        if(std::chrono::duration_cast<std::chrono::microseconds>(now - lastInstTime).count() < microSecPerInst)continue;
        if(std::chrono::duration_cast<std::chrono::seconds>(now - timerInsts).count() >= 1) {
            timerInsts = now;
            instCount = 0;
        }
        opcode = (mem[pc] << 8u) | mem[pc + 1]; pc+=2;
        
        (this->*opcodeTable[getCode(opcode)])(); 
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

    running = initSDL2() && initNcurses(); 
    return true;
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
    curs_set(0);
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
            (this->*opcodeTable[code])();
        
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
 * Calling this function assumes that ncurses was correctly initiated
 **/
void chip8::ncursesDeamon(chip8& chip) {
    WINDOW *regWin, *ramWin;
    int savedSize[2] = {chip.termSize[0],chip.termSize[1]};
    int regWinWidth = (float)savedSize[0] * 0.20;
    regWin = newwin(savedSize[1], regWinWidth, 0, 0);
    ramWin = newwin(savedSize[1], (float)savedSize[0] * 0.80, 0, regWinWidth + 1);
    uint16_t line;
    uint16_t tmpPc;
    while(chip.running) {
        // Handle resize
        getmaxyx(stdscr, chip.termSize[1], chip.termSize[0]);
        if(savedSize[0] != chip.termSize[0] || savedSize[1] != chip.termSize[1]) {
            savedSize[0] = chip.termSize[0];
            savedSize[1] = chip.termSize[1];
            regWinWidth = (float)savedSize[0] * 0.20;
            regWin = newwin(savedSize[1], regWinWidth, 0, 0);
            ramWin = newwin(savedSize[1], (float)savedSize[0] * 0.80, 0, regWinWidth + 1);
        }
        // Handle registers
        
        box(regWin, 0, 0);
        mvwprintw(regWin, 0, 0, "Registers");
        for(line = 1; line < 0xF && line < savedSize[1] - 1; ++line) {
            mvwprintw(regWin, line, 1, "v%1X  : 0x%2X", line ,  chip.registers[line]);
        }
        mvwprintw(regWin, line, 1, "dt  : 0x%2X", chip.delayTimer);line++;
        mvwprintw(regWin, line, 1, "st  : 0x%2X", chip.soundTimer);line++;
        line++;
        mvwprintw(regWin, line, 1, "PC  : 0x%2X", chip.pc);
        line++;
        mvwprintw(regWin, line, 1, "SP  : 0x%d", chip.sp);
        // Handle RAM
        box(ramWin, 0, 0);
        mvwprintw(ramWin, 0, 0, "RAM");
        ;
        for(line = 1, tmpPc = chip.pc; line < savedSize[1] - 1; line++, tmpPc+=2) {
            mvwprintw(ramWin, line, 1, "[$%3X] 0x%4X", tmpPc, (chip.mem[tmpPc] << 8) | (chip.mem[tmpPc + 1]));
        }
        wrefresh(regWin);
        wrefresh(ramWin);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    
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