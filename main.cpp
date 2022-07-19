#include "gbdisplay.h"
#include "gbprocessor.h"
#include <time.h>
int main(int argc, char** argv){
    GameboyDisplay gd;
    Memory mem(gd.gm.input);
    if(argc == 1){
        std::cout << "[DaMaGe] Input name of ROM you would like to play (no spaces): " << std::endl;
        std::string game;
        std::cin >> game;
        mem.load_cartridge(game);
    } else {
        for(int i = 1; i < argc; i++){
            if(std::string(argv[i]) == "-v" || std::string(argv[i]) == "-V"){
                std::cout << "DaMaGe - Original Gameboy Emulator" << std::endl;
                std::cout << "Version 1.0.0" << std::endl;
                return 0;
            } else {
                mem.load_cartridge(argv[i]);    
            }
        }
    }
    Clock clock(&mem);
    CPU cpu(&mem, &clock);
    
    gd.add_ppu(cpu.getppu());
    gd.gm.start();
    cpu.run();
    
    gd.gm.stopSDL();
    return EXIT_SUCCESS;
}