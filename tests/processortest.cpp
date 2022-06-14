#include "../gbprocessor.h"

int main(){
    Memory mem;
    mem.load_cartridge("sml.gb");
    Clock clock(&mem);
    timereg_t *timereg = mem.timereg;
    uint8_t *IE = mem.getref(0xFFFF);
    uint8_t *IF = mem.getref(0xFF0F);
    printf("%d, %d\n", *IE, *IF);
    CPU cpu(&mem, &clock);

}