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


/**
 * Render the part of the screen dedicated to RAM.
 * Will render term_height - 2 lines formatted this way :
 * [addr]
 **/
void chipMonitor::renderRam() {
    werase(winRam);
    box(winRam, 0, 0);
    // uint8_t line = 1;
    mvwprintw(winRam, 0, 0, "RAM");

    uint8_t** ram = chip->getRam();

    uint16_t pc = chip->getPc();
    size_t offset = lastRecordedSize[1] / 2;
    

    // Need this to keep addresses aligned
    if(pc%2==0) offset = offset%2 == 0 ? offset : offset - 1;
    else offset =  offset%2 == 0 ? offset - 1 : offset;

    
    uint16_t opcode;

    instruction_t currInst;

    // 100 overkill, will compute max size soon:)
    char currMemo[20];

    // Using 16 bits but are there any 255+ lines term?
    // Long but if I set tmpPc outside I have a warning, sorry
    for(uint16_t line = 1, tmpPc = std::max<uint16_t>(pc - offset, 0x200); line < lastRecordedSize[1] && tmpPc < 0xFFF; ++line, tmpPc+=2) {
        memset(currMemo, 0, 20);
        opcode =((*ram)[tmpPc] << 8) | ((*ram)[tmpPc + 1]);
        

        currInst = chip->getInstruction(opcode);

        switch(currInst.type) {

            case instruction_t::REG:
                sprintf(currMemo, currInst.mnemonic.c_str(), getX(opcode));
                break;
            
            case instruction_t::REG_REG:
                sprintf(currMemo, currInst.mnemonic.c_str(), getX(opcode), getY(opcode));
                break;

            case instruction_t::REG_BYTE:
                sprintf(currMemo, currInst.mnemonic.c_str(), getX(opcode), getKK(opcode));
                break;
            
            case instruction_t::REG_REG_NIBBLE:
                sprintf(currMemo, currInst.mnemonic.c_str(), getX(opcode), getY(opcode), getN(opcode));
                break;

            case instruction_t::ADDR:
                sprintf(currMemo, currInst.mnemonic.c_str(), getNNN(opcode));
                break;

            case instruction_t::NOP:
            case instruction_t::NO_ARG:
                memcpy(currMemo, currInst.mnemonic.c_str(), currInst.mnemonic.size());
                break;

            // Can't happen if chip->getInstruction(opcode) correctly called.
            case instruction_t::JP_TABLE:
                break;

        }

        // Duplicating lines but isn't it more efficient than 2 if ???
        if(tmpPc == pc) {
            wattron(winRam, COLOR_PAIR(W_BG_PAIR));
            mvwprintw(winRam, line, 1, "[$%03.3X] - ", tmpPc);
            mvwprintw(winRam, line, 10, currMemo, tmpPc, opcode);
            wattroff(winRam, COLOR_PAIR(1));
        } else{
            mvwprintw(winRam, line, 1, "[$%03.3X] - ", tmpPc);
            mvwprintw(winRam, line, 10, currMemo, tmpPc, opcode);
        }
    }


    
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