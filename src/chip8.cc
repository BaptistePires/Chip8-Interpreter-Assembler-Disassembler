/**
 * This interpreter is based on http://devernay.free.fr/hacks/chip8/C8TECH10.HTM spec.
 **/
#include <iostream>
#include <cstdint>

#define MEM_SIZE 4096
#define COUNT_REG 16
#define STACK_SIZE 16

struct chip8 {
    uint8_t registers[COUNT_REG];
    uint8_t soundTimer;
    uint8_t delayTimer;
    uint8_t sp;
    uint16_t I;
    uint16_t pc;
    uint16_t stack[STACK_SIZE];
    


};


int main(int argc, char** argv) {
    if(argc < 2) {
        std::cout << "Usage : " << argv[0] << " [file_path]" << std::endl;
        return 0;
    }
}