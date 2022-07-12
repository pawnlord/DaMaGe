#include "gbdisplay.h"
#include "gbprocessor.h"
#include <time.h>
int main(int argc, char** argv){
    GameboyDisplay gd;
    Memory mem(gd.gm.input);
    if(argc == 1){
        mem.load_cartridge("sml.gb");
    } else {
        mem.load_cartridge(argv[1]);    
    }
    Clock clock(&mem);
    CPU cpu(&mem, &clock);
    
    gd.add_ppu(cpu.getppu());
    gd.gm.start();
    cpu.run();
    
    gd.gm.stopSDL();
    return EXIT_SUCCESS;
}