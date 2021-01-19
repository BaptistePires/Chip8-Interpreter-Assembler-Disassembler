#include "chip8.hh"



chip8::chip8() {

    std::fill(std::begin(keyboard), std::end(keyboard), false);

    mem = new uint8_t[MEM_SIZE];
    pc = MEM_START;
    sp = 0;
    disassF = false;
    display = new uint32_t[DISPLAY_SIZE];
    memset(display, 0, sizeof(display[0]) * DISPLAY_SIZE);
    clockSpeed = INST_PER_SEC;
    
    // Load fonts
    // for(int i = 0; i < FONT_COUNT; ++i) mem[FONT_START_ADDR + i] = chip8_fontset[i];
    // for(int i = 0; i < FONT_COUNT; ++i){
    //     if((i!=0 && (i%10)==0)) std::cout << std::endl;
    //     std::cout  << "0x" << std::hex << (unsigned char) mem[FONT_START_ADDR + i] << " ";
    // }
    for(int i = 0; i < FONT_COUNT; ++i) mem[FONT_START_ADDR + i] = chip8_fontset[i];
    initFunctionsTable();



}

chip8::~chip8(){
    delete[] mem;
    delete[] display;

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

    std::ofstream log("logs.log");
    typedef std::chrono::time_point<std::chrono::high_resolution_clock> timer_t;
    

    log << clockSpeed << std::endl;
    double microSecPerInst = 1e6 / clockSpeed;
    
    // Test SDL Rendering
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cout << "Error while SDL_Init() :" << SDL_GetError() << std::endl;
        return;
    }

    SDL_Window *window;
    SDL_Renderer *renderer;
    if (SDL_CreateWindowAndRenderer(DISPLAY_WIDTH * 10, DISPLAY_HEIGHT * 10  , SDL_WINDOW_RESIZABLE |SDL_WINDOW_SHOWN, &window, &renderer) < 0) {
        std::cout << "Error while creating window and renderer : " << SDL_GetError() << std::endl;
    }
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, DISPLAY_WIDTH , DISPLAY_HEIGHT );
    if(texture < 0) {
        std::cout << "Error while creating texture" << std::endl;
        return;
    }
    SDL_Event e;

    timer_t lastInstTime = std::chrono::high_resolution_clock::now();
    timer_t now;
    while(running) {
                // SDL TEST
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
            

        now = std::chrono::high_resolution_clock::now();
        if(std::chrono::duration_cast<std::chrono::microseconds>(now - lastInstTime).count() < microSecPerInst)continue;
        opcode = (mem[pc] << 8u) | mem[pc + 1]; pc+=2;
        (this->*opcodeTable[getCode(opcode)])(); 
        // std::cout << "opcode : " << std::hex << "0x" << opcode << std::endl;   
        // for(int i = 0; i< 16;++i)
        //     if(keyboard[i]) std::cout << std::hex << i << " pressed" << std::endl;
        // render();

        SDL_UpdateTexture(texture, nullptr, &display[0], sizeof(display[0]) * DISPLAY_WIDTH);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        if(delayTimer>0) delayTimer--;
        if(soundTimer>0) soundTimer--;
        lastInstTime = std::chrono::high_resolution_clock::now();
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    
}

bool chip8::init() {

    running = true;
    return true;
}

void chip8::render() {
    
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