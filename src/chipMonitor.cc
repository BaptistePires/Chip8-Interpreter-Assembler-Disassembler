#include "chipMonitor.hh"

chipMonitor::chipMonitor(chip8 *c): chip(c) {
    resize();

    if((lastRecordedSize[0] < TERM_MIN_WIDTH) || (lastRecordedSize[1] < TERM_MIN_HEIGHT)){
        mvwprintw(stdscr, 0, 0, "Terminal has to be at least %dx%d", TERM_MIN_WIDTH, TERM_MIN_HEIGHT);
        wrefresh(stdscr);
    }
}

chipMonitor::~chipMonitor() {
    if(winRegisters != NULL) delwin(winRegisters);
    if(winRam != NULL) delwin(winRam);
}


// 
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
        renderKeyboard();

        
        wrefresh(winRegisters);
        wrefresh(winRam);
        wrefresh(winKeybr);
        
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
    for(uint16_t line = 2, tmpPc = std::max<uint16_t>(pc - offset, 0x200); (line < lastRecordedSize[1]-2) && tmpPc < 0xFFF; ++line, tmpPc+=2) {
        memset(currMemo, 0, 20);
        opcode =((*ram)[tmpPc] << 8) | ((*ram)[tmpPc + 1]);
        

        currInst = chip->getInstruction(opcode);

        // Format line based on instruction type
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
            mvwprintw(winRam, line, 2, ">[$%03.3X] - ", tmpPc);
            mvwprintw(winRam, line, 11, currMemo, tmpPc, opcode);
            wattroff(winRam, COLOR_PAIR(1));
        } else{
            mvwprintw(winRam, line, 2, "[$%03.3X] - ", tmpPc);
            mvwprintw(winRam, line, 11, currMemo, tmpPc, opcode);
        }
    }
    
}

void chipMonitor::renderRegisters() {
    box(winRegisters, 0, 0);
    mvwprintw(winRegisters, 0, 0, "Registers");
    int currWinSize[2];
    getmaxyx(winRegisters, currWinSize[1], currWinSize[0]);
    
    uint8_t line = 2 ;
    uint8_t col = 2;
    uint8_t *registers = chip->getRegisters();
    for(uint8_t r = 0; r < 0xF ; ++line, r++){
        if(line == (currWinSize[1] - 2)) {
            col += 12;
            line = 2;
            if(col >= currWinSize[0]) return;
        }
        mvwprintw(winRegisters, line, col, "V%01.1X : 0x%02.2X", r, registers[r]);
    }
    
    line++;

    mvwprintw(winRegisters, line++, col, "DT : 0x%02.2X", chip->getDT());
    mvwprintw(winRegisters, line++, col, "ST : 0x%02.2X", chip->getST());
    mvwprintw(winRegisters, line++, col, "SP : 0x%02.2X", chip->getSP());
}

void chipMonitor::renderKeyboard() {
    #define SET_ATTR_IF_KEY(key) { \
        if(keyboard[key]) { \
            wattron(winKeybr, COLOR_PAIR(W_BG_PAIR)); \
            mvwprintw(winKeybr,line, col, " %01.1X ", key); \
            wattroff(winKeybr, COLOR_PAIR(W_BG_PAIR));\
            \
        } else \
            mvwprintw(winKeybr,line, col, " %01.1X ", key); \
        col+= 3; \
    }
     
    box(winKeybr, 0, 0);
    mvwprintw(winKeybr, 0, 0, "Keyboard");
    std::atomic<bool>* keyboard = chip->getKeyboard();

    uint8_t line = 2;
    uint8_t col = 2;
    mvwprintw(winKeybr, line++, col, "+---------------+");
    mvwprintw(winKeybr, line, col++, "|");

    SET_ATTR_IF_KEY(0x1);
    mvwprintw(winKeybr, line, col++, "|");
    SET_ATTR_IF_KEY(0x2);
    mvwprintw(winKeybr, line, col++, "|");
    SET_ATTR_IF_KEY(0x3);
    mvwprintw(winKeybr, line, col++, "|");
    SET_ATTR_IF_KEY(0xC);
    waddch(winKeybr, '|');

    mvwprintw(winKeybr, ++line, (col=2), "+---------------+");
    
    mvwprintw(winKeybr, ++line, col++, "|");

    SET_ATTR_IF_KEY(0x4);
    mvwprintw(winKeybr, line, col++, "|");
    SET_ATTR_IF_KEY(0x5);
    mvwprintw(winKeybr, line, col++, "|");
    SET_ATTR_IF_KEY(0x6);
    mvwprintw(winKeybr, line, col++, "|");
    SET_ATTR_IF_KEY(0xD);
    mvwprintw(winKeybr, line, col++, "|");

    mvwprintw(winKeybr, ++line, (col=2), "+---------------+");
    
    mvwprintw(winKeybr, ++line, col++, "|");

    SET_ATTR_IF_KEY(0x7);
    mvwprintw(winKeybr, line, col++, "|");
    SET_ATTR_IF_KEY(0x8);
    mvwprintw(winKeybr, line, col++, "|");
    SET_ATTR_IF_KEY(0x9);
    mvwprintw(winKeybr, line, col++, "|");
    SET_ATTR_IF_KEY(0xE);
    mvwprintw(winKeybr, line, col++, "|");

    mvwprintw(winKeybr, ++line, (col=2), "+---------------+");
    
    mvwprintw(winKeybr, ++line, col++, "|");

    SET_ATTR_IF_KEY(0xA);
    mvwprintw(winKeybr, line, col++, "|");
    SET_ATTR_IF_KEY(0x0);
    mvwprintw(winKeybr, line, col++, "|");
    SET_ATTR_IF_KEY(0xB);
    mvwprintw(winKeybr, line, col++, "|");
    SET_ATTR_IF_KEY(0xF);
    mvwprintw(winKeybr, line, col++, "|");

    mvwprintw(winKeybr, ++line, (col=2), "+---------------+");
    
    #undef SET_ATTR_IF_KEY
}

/**
 * Resize windows if needed.
 * Returns always true unless an error happened (call to newwin failled).
 */
bool chipMonitor::resize() {
    int tmpSize[2];
    getmaxyx(stdscr, tmpSize[1], tmpSize[0]);
    if((tmpSize[0] == lastRecordedSize[0]) && (tmpSize[1] == lastRecordedSize[1])) return true;
    lastRecordedSize[0] = tmpSize[0];
    lastRecordedSize[1] = tmpSize[1];
    
    return createWindows();
}

bool chipMonitor::createWindows() {
    
    if((winRegisters = newwin(lastRecordedSize[1], REG_WIN_SIZE, 0, 0)) == NULL) {
        std::cout << "Error while creating window with ncurses." << std::endl;
        return false;
    }

    if((winRam = newwin(lastRecordedSize[1], RAM_WIN_SIZE, 0, REG_WIN_SIZE + 1)) == NULL) {
        std::cout << "Error while creating window with ncurses." << std::endl;
        delwin(winRegisters);
        return false;
    }

    if((winKeybr = newwin(lastRecordedSize[1], KEYBOARD_WIN_SIZE, 0, REG_WIN_SIZE + RAM_WIN_SIZE + 1)) == NULL) {
        std::cout << "Error while creating window with ncurses." << std::endl;
        delwin(winRegisters);
        delwin(winRam);
        return false;
    }


    return true;
}