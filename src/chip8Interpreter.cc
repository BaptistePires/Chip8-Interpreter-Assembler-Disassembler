/**
 * This interpreter is based on http://devernay.free.fr/hacks/chip8/C8TECH10.HTM spec.
 * 
 * Roms can be found here : https://www.zophar.net/pdroms/chip8/chip-8-games-pack.html.
 **/
#include <iostream>
#include "chip8.hh"



int main(int argc, char** argv) {
    if(argc < 2) {
        std::cout << "Usage : " << argv[0] << " [file_path]" << std::endl;
        return 0;
    }

    chip8 c;
    if(!c.loadFile(argv[1])) return 0;
    c.run();

    std::cout << "See you" << std::endl;

}