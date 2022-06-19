#include "../gbprocessor.h"

int main(int argc, char** argv){
    Memory mem;
    if(argc == 1){
        mem.load_cartridge("sml.gb");
    } else {
        mem.load_cartridge(argv[1]);    
    }
    Clock clock(&mem);
    timereg_t *timereg = mem.timereg;
//    timereg->TAC = 4;
    uint8_t *IE = mem.getref(0xFFFF);
    uint8_t *IF = mem.getref(0xFF0F);
    printf("%d, %d\n", *IE, *IF);
    CPU cpu(&mem, &clock);
    cpu.run();

}