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
                    {"LD I, $%02.2X", instruction_t::type_t::ADDR, &chip8::ocANNN},
                    {"JP V0, $%03.3X", instruction_t::type_t::ADDR, &chip8::ocBNNN},
                    {"RND V%01.1X, $%02.2X", instruction_t::type_t::REG_BYTE, &chip8::ocCXKK},
                    {"DRW V%01.1X, V%01.1X, $%01.1X", instruction_t::type_t::REG_REG_NIBBLE, &chip8::ocDXYN},
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
    if(isMonitoring)monitor = nullptr;
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
    std::thread* t;
    if(isMonitoring) t = new std::thread(&chipMonitor::render, monitor);
    
    
    while(running) {

        handleEvents();
        if(halt) continue;
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
        // SDL_PauseAudio(SDL_FALSE);
        if(soundTimer>0){
            soundTimer--;
            SDL_PauseAudio(SDL_FALSE);
        }else{
            SDL_PauseAudio(SDL_TRUE);
        }
        // std::cout << "\r R3 : " << (int) (registers[3]) << " Inst. per sec :" << std::dec << std::flush << instCount <<"." << std::flush;
        lastInstTime = std::chrono::high_resolution_clock::now();
    }
    
    if(isMonitoring)t->join();

}

bool chip8::init() {

    

    if(isMonitoring){
        if(initNcurses()) {
            monitor = new chipMonitor(this);
        }else{
            std::cout << "Can't run monitor. Please check that you have the requiered ncurses version installed.(see README.md)" << std::endl; 
        }
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
    init_pair(W_BG_PAIR, COLOR_BLACK, COLOR_WHITE);
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


instruction_t* chip8::getInstruction(uint16_t opcode) {
    uint8_t code = getCode(opcode);
    instruction_t* i = &instructionTable[code];

    // Handle indirection
    if(i->type == instruction_t::type_t::JP_TABLE) {
            switch(code) {
                case 0x0:
                    i = &instructionTable0[getN(opcode)];
                    break;
                
                case 0x8:
                    i = &instructionTable8[getN(opcode)];
                    break;

                case 0xE:
                    i = &instructionTableE[getN(opcode)];
                    break;

                case 0xF:
                    i = &instructionTable0[getKK(opcode)];
                    break;
            }
    }
    return i;
}

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

uint8_t** chip8::getRam() {
    return &mem;
}

uint16_t chip8::getPc() {
    return pc;
}

uint8_t chip8::getDT() {
    return delayTimer;
}

uint8_t chip8::getST() {
    return soundTimer;
}
uint8_t chip8::getSP() {
    return sp;
}

std::atomic<bool>* chip8::getKeyboard() {
    return keyboard;
}

uint16_t chip8::getI() {
    return I;
}

void chip8::setMonitoring(bool f) {
    isMonitoring = f;
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

            case SDLK_SPACE:
                halt = !halt;
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