#include "gbmem.h"

void print_cart_info(cart_info* inf) {
    std::cout << "Entry: " << std::hex << inf->entry_point << std::endl;
    std::cout << "Title: " << std::string((char*) inf->title) << std::endl;
}

Memory::Memory(){
    raw_mem = (uint8_t*) malloc(sizeof(uint8_t) * 0x10000);
    timereg = (timereg_t*)(raw_mem+0xFF04); // registers for dividers and timers 
}
cart_info* Memory::load_cartridge(std::string filename){
    std::ifstream ifs (filename, std::ifstream::binary);
    std::filebuf* pbuf = ifs.rdbuf();
    std::size_t size = pbuf->pubseekoff (0,ifs.end,ifs.in);
    pbuf->pubseekpos (0,ifs.in);
    pbuf->sgetn((char*)raw_mem, size);
    
    // load cartridge into inf
    std::memcpy(&inf, raw_mem+0x100, sizeof(cart_info));
    print_cart_info(&inf);
    return &inf;
}


uint8_t* Memory::getref(uint16_t addr){
    return raw_mem+addr;
}

void Memory::req_int(uint8_t flags){
    raw_mem[0xFF0F] |= flags;
}
