#include "../gbmem.h"

int main(){
    Memory mem;
    std::cout << "Loading" << std::endl;
    mem.load_cartridge("sml.gb");
    std::cout << "Tests Done" << std::endl;
}