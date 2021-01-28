#ifndef CHIP_MONITOR_HEDEAR
#define CHIP_MONITOR_HEDEAR

#include "chip8.hh"

class chip8;
/**
 * This clas is used to monitor the state of the interpreter.
 * Whenever it's instantiated it assumes that ncurses is 
 * already instantiated and ready to be used. 
 **/
class chipMonitor  {
    chip8 *chip;
    WINDOW *winRegisters;
    WINDOW *winRam;

    // 0 width, 1 height
    int lastRecordedSize[2]{-1, -1};

    public:
        chipMonitor(chip8* c);    
        ~chipMonitor();

        void render();
        void renderRam();
        void renderRegisters();
        bool resize();

        bool createWindows();
};



#endif