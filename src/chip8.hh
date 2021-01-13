#ifndef CHIP8_HEADER
#define CHIP8_HEADER

#include <iostream>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <istream>
#include <algorithm>
#include <random>

#define MEM_SIZE 4096
#define COUNT_REG 16
#define STACK_SIZE 16
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define DISPLAY_SIZE DISPLAY_WIDTH * DISPLAY_HEIGHT
#define MEM_START 0x200

#define getNNN(opcode) (opcode & 0x0FFF)
#define getKK(opcode) (opcode & 0x00FF)
#define getX(opcode) (opcode & 0x0F00)
#define getY(opcode) (opcode & 0x00F0)
#define getN(opcode) (opcode & 0x000F)



class chip8 {
    uint8_t registers[COUNT_REG];
    uint8_t soundTimer;
    uint8_t delayTimer;
    uint8_t sp;
    uint16_t I;
    uint16_t pc;

    uint8_t *mem;
    uint16_t stack[STACK_SIZE];
    bool keyboard[0xF];
    uint8_t display[DISPLAY_SIZE];

    uint16_t opcode;
    bool waitingKey;
    bool debug = true;
    public:
        chip8();
        ~chip8(); 

        bool loadFile(std::string&& filepath);
        void run();

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
        
};

#endif