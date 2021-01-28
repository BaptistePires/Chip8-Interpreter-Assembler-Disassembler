#ifndef CHIP_MONITOR_HEDEAR
#define CHIP_MONITOR_HEDEAR

#include "chip8.hh"

const int TERM_MIN_WIDTH = 120;
const int TERM_MIN_HEIGHT = 30;
const int REG_WIN_SIZE = 27;
const int RAM_WIN_SIZE = 40;
const int KEYBOARD_WIN_SIZE = 21;
const int KEYBOARD_WIN_HEIGHT = 13;



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
    WINDOW *winKeybr;

    // 0 width, 1 height
    int lastRecordedSize[2]{-1, -1};

    void renderRam();
    void renderRegisters();
    void renderKeyboard();
    bool resize();

    bool createWindows();


    public:
        chipMonitor(chip8* c);    
        ~chipMonitor();

        void render();

};


#endif