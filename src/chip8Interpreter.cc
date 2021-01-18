/**
 * This interpreter is based on http://devernay.free.fr/hacks/chip8/C8TECH10.HTM spec.
 * 
 * Roms can be found here : https://www.zophar.net/pdroms/chip8/chip-8-games-pack.html.
 * 
 * You can also disassamble chip8 files. 
 **/
#include <iostream>
#include <map>
#include <string>
#include "chip8.hh"


const std::string DISASS_FLAG = "-d"; 
const std::string DISASS_FLAG2 = "--disass"; 

typedef std::map<std::string, bool> opts_t;
opts_t setUpOpts(int optsCount, char**& args);
bool isOptSet(opts_t& opts, const std::string& opt);

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cout << "Usage : " << argv[0] << " [file_path]" << std::endl;
        return 0;
    }

    opts_t opts =setUpOpts(argc, argv);

    chip8 c;
    


    if(isOptSet(opts, DISASS_FLAG) || isOptSet(opts, DISASS_FLAG2)) {
        if(!c.loadFile(argv[argc - 1])) return 0;
        c.disass();
    }else{
        if(!c.loadFile(argv[1])) return 0;
        c.run();

        std::cout << "See you" << std::endl;
    }

    

}

opts_t setUpOpts(int optsCount, char**& args) {
    opts_t opts;
    for(int i = 0; i < optsCount; ++i) opts[args[i]] = true;
    return opts;
}

bool isOptSet(opts_t& opts, const std::string& opt) {
    return opts.find(opt) != opts.end();
}