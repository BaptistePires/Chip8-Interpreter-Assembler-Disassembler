#ifndef CHIP8_HEADER
#define CHIP8_HEADER

#include <iostream>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <istream>
#include <algorithm>
#include <random>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <bitset>
#include <cstdlib>
#include <sstream>
#include <atomic>
#include <cmath>
#include <ncurses.h>
#include <SDL2/SDL.h>


#define W_BG_PAIR 1
#define B_BG_PAIR 2

#define MEM_SIZE 4096
#define COUNT_REG 16
#define STACK_SIZE 16
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define DISPLAY_SIZE DISPLAY_WIDTH * DISPLAY_HEIGHT
#define MEM_START 0x200
#define PC_START 0x200
#define FONT_COUNT 80
#define FONT_START_ADDR 0x000

#define INST_PER_SEC 100

#define getNNN(opcode) (opcode & 0x0FFF)
#define getKK(opcode) (opcode & 0x00FF)
#define getX(opcode) ((opcode & 0x0F00) >> 8u)
#define getY(opcode) ((opcode & 0x00F0) >> 4u)
#define getN(opcode) (opcode & 0x000F)
#define getCode(opcode) ((opcode & 0xF000) >> 12u)


struct rendererWrapper {
    SDL_Window *w;
    SDL_Renderer *r;
    SDL_Texture* texture;
};

struct audioWrapper {
    SDL_AudioSpec spec;
    SDL_AudioDeviceID device;
    int sampleNo;
};

typedef rendererWrapper rendererWrapper_t;
typedef audioWrapper audioWrapper_t;


class chip8 {
    // As it's used by the monitor thread as reading only, avoiding mutexes
    uint8_t registers[COUNT_REG];
    uint8_t soundTimer;
    uint8_t delayTimer;
    uint8_t sp;
    uint16_t I;
    uint16_t pc;
    int fontStartAddr = FONT_START_ADDR;

    uint8_t *mem;
    uint16_t stack[STACK_SIZE];
    std::atomic<bool> keyboard[0xF];
    // Probably will use a bitset next don't know yet if it's worth, it's a """small""" array anyway
    uint32_t *display;
    rendererWrapper_t rendererWrapper;
    audioWrapper_t audio;
    std::string romPath;
    
    double clockSpeed;
    int termSize[2];
    uint16_t opcode;
    bool debug = true;  
    std::atomic<bool> running, needRender;
    

    // Used to disass
    bool disassF;
    size_t fSize;
    std::ofstream disassFile;
    

    // Functions tables
    void (chip8::*opcodeTable[0xF + 1])();
    void (chip8::*ocTable0[0xE + 1])();
    void (chip8::*ocTable8[0xE + 1])();
    void (chip8::*ocTableE[0xE + 1])();
    void (chip8::*ocTableF[0x65 + 1])();

    std::vector<uint8_t> opcodesF = {0x07, 0x0A, 0x15, 0x18, 0x1E, 0x20, 0x33, 0x55, 0x65};

    public:
        chip8();
        ~chip8(); 

        bool loadFile(std::string&& filepath);
        void run();
        void disass();
        void setClock(double clock);
        void setFontAddr(int fontAddr);

    private:
        bool init();
        bool initSDL2();
        bool initNcurses();
        
        void render();
        void initFunctionsTable();
        void handleEvents();
        static void ncursesDeamon(chip8& chip);

        void tableOc0();
        void oc00E0();
        void oc00EE();

        void oc1NNN();
        void oc2NNN();
        void oc3XKK();
        void oc4XKK();
        void oc5XY0();
        void oc6XKK();
        void oc7XKK();

        void tableOc8();
        void oc8XY0();
        void oc8XY1();
        void oc8XY2();
        void oc8XY3();
        void oc8XY4();
        void oc8XY5();
        void oc8XY6();
        void oc8XY7();
        void oc8XYE();

        void oc9XY0();
        void ocANNN();
        void ocBNNN();
        void ocCXKK();
        void ocDXYN();

        void tableOcE();
        void ocEX9E();
        void ocEXA1();

        void tableOcF();
        void ocFX07();
        void ocFX0A();
        void ocFX15();
        void ocFX18();
        void ocFX1E();
        void ocFX29();
        void ocFX33();
        void ocFX55();
        void ocFX65();

    private:
        // Took it from http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
        uint8_t chip8_fontset[FONT_COUNT] = { 
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        };
        
};

#endif