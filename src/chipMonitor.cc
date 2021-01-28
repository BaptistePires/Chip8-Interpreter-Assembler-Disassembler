#include "chipMonitor.hh"

chipMonitor::chipMonitor(chip8 *c): chip(c) {
    resize();
}

chipMonitor::~chipMonitor() {
    if(winRegisters != NULL) delwin(winRegisters);
    if(winRam != NULL) delwin(winRam);
}


void chipMonitor::render(){
    std::atomic<bool> *running = chip->getRunning(), *halt = chip->getHalt();

    while(*running) {
        resize();
        do {
            if(wgetch(stdscr) == 10){
                *halt = !(*halt);
                std::this_thread::yield();
                continue;
            }

        }while((*halt) && (*running));
        
        
        renderRegisters();
        renderRam();

        
        wrefresh(winRegisters);
        wrefresh(winRam);
        
    }
}



void chipMonitor::renderRam() {
    box(winRam, 0, 0);
    // uint8_t line = 1;
    mvwprintw(winRam, 0, 0, "RAM");

    
}

void chipMonitor::renderRegisters() {
    box(winRegisters, 0, 0);
    mvwprintw(winRegisters, 0, 0, "Registers");
    uint8_t line = 1;
    uint8_t *registers = chip->getRegisters();
    for(; line < 0xF && line < (lastRecordedSize[1] - 1); ++line)
        mvwprintw(winRegisters, line, 1, "v%01.1X  : 0x%02.2X", (line - 1), registers[line - 1]);

    
}

/**
 * Resize windows if needed.
 * Returns always true unless an error happened (call to newwin failled).
 */
bool chipMonitor::resize() {
//     regWin = newwin(savedSize[1], regWinWidth, 0, 0);
//     ramWin = newwin(savedSize[1], (float)savedSize[0] * 0.80, 0, regWinWidth + 1);
    int tmpSize[2];
    getmaxyx(stdscr, tmpSize[1], tmpSize[0]);
    if((tmpSize[0] == lastRecordedSize[0]) && (tmpSize[1] == lastRecordedSize[1])) return true;
    lastRecordedSize[0] = tmpSize[0];
    lastRecordedSize[1] = tmpSize[1];
    
    return createWindows();;
}

bool chipMonitor::createWindows() {
    int regWinWidth = (float)lastRecordedSize[0] * .20;
    if((winRegisters = newwin(lastRecordedSize[1], regWinWidth, 0, 0)) == NULL) {
        std::cout << "Error while creating window with ncurses." << std::endl;
        return false;
    }

    if((winRam = newwin(lastRecordedSize[1], (float)lastRecordedSize[0] * 0.80, 0, regWinWidth + 1)) == NULL) {
        std::cout << "Error while creating window with ncurses." << std::endl;
        delwin(winRegisters);
        return false;
    }

    return true;
}