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
const std::string CLOCK_FLAG = "-c";

/**
 *  Type of options:
 *      - key   : flag used as std::string
 *      - value : 
 *          - first  : If it's set, if it's in the map it should but who knows
 *          - second : index of the flag, used to retrieve values
 **/
typedef std::map<std::string, std::pair<bool, int>> opts_t;
opts_t setUpOpts(int optsCount, char**& args);
bool isOptSet(opts_t& opts, const std::string& opt);

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cout << "Usage : " << argv[0] << " [file_path]" << std::endl;
        return 0;
    }

    opts_t opts =setUpOpts(argc, argv);

    chip8 c;
    
    if(isOptSet(opts, DISASS_FLAG)) {
         std::pair<bool, int> opt = opts[DISASS_FLAG];
        if(!c.loadFile(argv[opt.second + 1])) return 0;
        c.disass();
    }else{

        if(isOptSet(opts, CLOCK_FLAG)) {
            std::pair<bool, int> opt = opts[CLOCK_FLAG];
            double clock = atoi(argv[opt.second + 1]);
            
            if(clock > 0) c.setClock(clock);
        }   

        if(!c.loadFile(argv[1])) return 0;
        c.run();

        std::cout << "See you" << std::endl;
    }

    

}

opts_t setUpOpts(int optsCount, char**& args) {
    opts_t opts;
    for(int i = 0; i < optsCount; ++i)opts[args[i]] = std::make_pair(true, i);
    return opts;
}

bool isOptSet(opts_t& opts, const std::string& opt) {
    return opts.find(opt) != opts.end();
}