#include "gbmem.h"

void print_cart_info(cart_info* inf) {
    std::cout << "Entry: " << std::hex << inf->entry_point << std::endl;
    std::cout << "Title: " << std::string((char*) inf->title) << std::endl;
}

Memory::Memory(){
    raw_mem = (uint8_t*) malloc(sizeof(uint8_t) * 0x10000);
    timereg = (timereg_t*)(raw_mem+0xFF04); // registers for dividers and timers 
}


void Memory::reset_regs(){
    set(0xCF, kP1);
    set(0x00, kSB);
    set(0x7E, kSC);
    set(0xAB, kDIV);
    set(0xAB, kDIV);
    set(0, kTIMA);
    set(0, kTMA);
    set(0xF8, kTAC);
    set(0xE1, kIF);
    set(0x80, kNR1(0));
    set(0x80, kNR1(0));
    set(0x80, kNR1(0));
    set(0x80, kNR1(0));
    set(0x80, kNR1(0));
    set(0x80, kNR2(0));
    set(0x80, kNR2(0));
    set(0x80, kNR2(0));
    set(0x80, kNR2(0));
    set(0x80, kNR3(0));
    set(0x80, kNR3(0));
    set(0x80, kNR3(0));
    set(0x80, kNR3(0));
    set(0x80, kNR3(0));
    set(0x80, kNR4(0));
    set(0x80, kNR4(0));
    set(0x80, kNR4(0));
    set(0x80, kNR4(0));
    set(0x80, kNR5(0));
    set(0x80, kNR5(0));
    set(0x80, kNR5(0)); // TODO
    set(0x91, kLCDC);
    set(0x85, kSTAT);
    set(0, kSCY);
    set(0, kSCX);
    set(0, kLY);
    set(0, kLYC);
    set(0xFF, kDMA);
    set(0xFC, kBGP);
    set(0, kOBP0);
    set(0, kOBP1);
    set(0, kWY);
    set(0, kWX);
    set(0xFF, kKEY1);
    set(0, kIE);
}

uint8_t Memory::get(uint16_t idx){
    return raw_mem[idx];
}

uint8_t& Memory::operator[](int idx){
    return raw_mem[idx];
}

void Memory::set(uint8_t v, uint16_t addr){
    raw_mem[addr] = v;
}

cart_info* Memory::load_cartridge(std::string filename){
    std::ifstream ifs (filename, std::ifstream::binary);
    std::filebuf* pbuf = ifs.rdbuf();
    std::size_t size = pbuf->pubseekoff (0,ifs.end,ifs.in);
    pbuf->pubseekpos (0,ifs.in);
    pbuf->sgetn((char*)raw_mem, size);
    
    // load cartridge into inf
    std::memcpy(&inf, raw_mem+0x100, sizeof(cart_info));
    reset_regs();
    print_cart_info(&inf);
    return &inf;
}


uint8_t* Memory::getref(uint16_t addr){
    return raw_mem+addr;
}

void Memory::req_int(uint8_t flags){
    raw_mem[0xFF0F] |= flags;
}

bool Memory::get_int(uint8_t flags){
    return (raw_mem[0xFF0F] & flags) > 0;
}
void Memory::reset_int(uint8_t flags){
    raw_mem[0xFF0F] &= ~(flags);
}
uint8_t Memory::get_int_num(){
    for(int i = 0; i < 5; i++){
        if(raw_mem[0xFF0F] & pow(2, i) > 0){
            return i;
        }
    }
    return -1;
}
