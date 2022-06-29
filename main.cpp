#include "gbdisplay.h"
#include "gbprocessor.h"
#include <time.h>
int main(int argc, char** argv){
    Memory mem;
    if(argc == 1){
        mem.load_cartridge("sml.gb");
    } else {
        mem.load_cartridge(argv[1]);    
    }
    Clock clock(&mem);
    CPU cpu(&mem, &clock);
    
    GameboyDisplay gd(cpu.getppu());

    gd.gm.start();
    cpu.run();
    
    gd.gm.stopSDL();
    return EXIT_SUCCESS;
}