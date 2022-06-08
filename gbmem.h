#ifndef GBMEM_H
#define GBMEM_H
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cstring>

struct cart_info{
    int32_t entry_point;
    int8_t logo[0x30];
    int8_t title[0x10];
    int8_t CGB_flag;
    int16_t new_license_code;
    int8_t SGB_flag;
    int8_t MBC_type;
    int8_t ROM_size;
    int8_t RAM_size;
    int8_t dest_code;
    int8_t old_license_code;
    int8_t vers_no;
    int8_t head_cs;
    int8_t glob_cs;
};

void print_cart_info(cart_info* inf);

class Memory{
    public:
    Memory();
    char get(uint16_t addr);
    void set(uint8_t v, uint16_t addr);
    cart_info* load_cartridge(std::string filename);
    private:
    uint8_t* raw_mem;
    cart_info inf;
};

#endif